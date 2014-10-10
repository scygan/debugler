/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma warning(disable : 4996)

#include "process.h"

#ifdef _WIN32
#define snprintf _snprintf
#else
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <cstdio>
#include <stdint.h>

#pragma warning(push)
//'boost::program_options::options_description':
//assignment operator could not be generated:
#pragma warning(disable : 4512)
#include <boost/program_options/options_description.hpp>
#pragma warning(pop)
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>

#include <DGLCommon/os.h>
#include <DGLCommon/wa.h>
#include <DGLCommon/ipc.h>
#include <DGLCommon/version.h>

#include <DGLInject/inject.h>

#ifdef __ANDROID__
#include <libgen.h>
#include <dlfcn.h>
#define DGL_SOCKET_PROP ("debug." DGL_PRODUCT_LOWER ".socket")

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#endif

#pragma warning(disable : 4503)

struct IPCMessage {
    IPCMessage() : m_ok(true) {
        m_message[0] = 0;
    };
    int8_t m_message[1000];
    int8_t m_ok;
};

std::string getWrapperPath() {
#ifdef _WIN32
#ifdef _WIN64
    std::string ret = "DGLWrapper64\\DGLWrapper.dll";
#else
    std::string ret = "DGLWrapper\\DGLWrapper.dll";
#endif

    char fileName[DGL_MAX_PATH];
    if (!GetModuleFileName(NULL, fileName, DGL_MAX_PATH)) {
        return ret;
    }
    std::string tmp = fileName;
    size_t splitPoint = tmp.find_last_of("/\\");
    return tmp.substr(0, splitPoint) + "\\" + ret;
#elif defined(__ANDROID__)
    return "/system/lib/libdglwrapper.so";
#else
    return "libdglwrapper.so";
#endif
}

namespace po = boost::program_options;
using namespace std;

int main(int argc, char** argv) {
#ifdef __ANDROID__
        //Disable SELinux - otherwise it will kill us on Android < 4.4
        {
            struct stat S;
            if (stat("/sys/fs/selinux/enforce", &S) == 0) {
                std::ofstream enforce("/sys/fs/selinux/enforce");
                enforce << "0\n";
                if (!enforce.good()) {
                    Os::info("Cannot disable SELinux");
                } else {
                    Os::info("SELinux disabled");
                }
            }
        }
#endif

    IPCMessage message;
    IPCMessage* ipcMessage = &message;

    DGLProcess::native_process_handle_t childHandle = 0;

    try {

        po::options_description desc(
            DGL_PRODUCT + getVersion() + " (dglloader)" + 
            "\nThe OpenGL(R) debugger\n\n"
            "Copyright (C) 2013 " DGL_MANUFACTURER ".\n\n "
            "https://github.com/debugler/debugler\n\n"
            "Allowed options:");

        desc.add_options()("help,h", "produce help message")(
                "egl", "use egl mode")("nowait",
                                       "do not wait for debugger to connect");

        desc.add_options()("port", po::value<vector<string> >(),
                           "Debugger TCP port number.");

        desc.add_options()("skip", po::value<vector<int> >(),
            "Number of processes to skip.");

        po::options_description mandatory("Mandatory options");
        mandatory.add_options()(
                "execute", po::value<vector<string> >()->composing(),
                "command to execute. Use -- top supply additional args.");

        po::positional_options_description p;
        p.add("execute", -1);

        po::options_description all;
        all.add(desc).add(mandatory);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
                          .options(all)
                          .positional(p)
                          .run(),
                  vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::ostringstream out;
            out << desc << "\n" << mandatory << "\n";
            throw std::runtime_error(out.str());
        }

        if (!vm.count("execute")) {
            std::ostringstream out;
            out << "Nothing to execute.\n" << desc << "\n" << mandatory << "\n";
            throw std::runtime_error(out.str());
        }

        std::string executable = vm["execute"].as<vector<string> >()[0];
        std::vector<std::string> arguments;

        for (size_t i = 0; i < vm["execute"].as<vector<string> >().size();
             i++) {
            arguments.push_back(vm["execute"].as<vector<string> >()[i]);
        }

        std::string wrapperPath = getWrapperPath();
        Os::info("Executable: %s\nWrapper: %s\n\n\n", executable.c_str(),
                 wrapperPath.c_str());


        DGLIPC::DebuggerMode debuggerMode = DGLIPC::DebuggerMode::DEFAULT;
        DGLIPC::DebuggerListenMode debuggerListenMode = DGLIPC::DebuggerListenMode::LISTEN_AND_WAIT;
        DGLIPC::DebuggerPortType debuggerPortType = DGLIPC::DebuggerPortType::TCP;
        std::string debuggerPortName = "5555";

        if (vm.count("egl")) {
            debuggerMode = (DGLIPC::DebuggerMode::EGL);
        }
        
        if (vm.count("nowait")) {
            debuggerListenMode = DGLIPC::DebuggerListenMode::LISTEN_NO_WAIT;
        }

        if (vm.count("port")) {
            std::string portStr = vm["port"].as<vector<string> >()[0];
            if (portStr.find("unix:") == 0) {
                std::string portPath = portStr.substr(strlen("unix:"));
#ifdef __ANDROID__
                // On Android we do a small WA here: we expect app may not have
                // enough permissions to
                // write it's socket, so we do a magic chmod here (as we have
                // probably more access right than app)
                std::string portPathCopy =
                        portPath;    // dirname may modify the passed string
                if (chmod(dirname(portPathCopy.c_str()), 0777) != 0) {
                    throw std::runtime_error(
                            std::string(
                                    "Cannot change permission to unix socket "
                                    "directory: ") +
                            Os::translateOsError(Os::getLastosError()));
                }

                // On Android we want to leave a trace of unix socket path, so
                // GUI knows where to look for sockets.
                Os::setProp(DGL_SOCKET_PROP, portPath.c_str());
#endif
                debuggerPortType =  DGLIPC::DebuggerPortType::UNIX;
                debuggerPortName =  portPath;
            } else if (portStr.find("tcp:") == 0) {
                debuggerPortName =  portStr.substr(strlen("tcp:"));
            } else {
                debuggerPortName =  portStr;
            }
        }

        int numberOfSkippedProcesses = 0;

        if (vm.count("skip")) {
            numberOfSkippedProcesses = vm["skip"].as<vector<int> >()[0];
        }

        std::shared_ptr<DGLIPC> dglIPC = DGLIPC::Create(wrapperPath, debuggerMode,
            debuggerListenMode, debuggerPortType, debuggerPortName.c_str(), numberOfSkippedProcesses);

#ifdef __ANDROID__
        // If we are running in processtree spawned by another  dglloader
        // instance
        // this variable will be non-zero length.
        // On Android we have to catch that, to setup proper dl-interception
        std::string oldUUID = Os::getEnv("dgl_uuid");
#endif

        Os::setEnv("dgl_uuid", dglIPC->getUUID().c_str());

        DGLInject inject(wrapperPath);

        DGLProcess process(executable, arguments);

        if (process.getHandle() == 0) {

            inject.injectPostFork();
// we are in forked out process
#ifdef _WIN32
            DGL_ASSERT(!"forked out on windows");
#else

#ifdef __ANDROID__
            {
                // On Android this is the last change to get pointers required
                // for dl-interception.
                // Either copy them form existing dglloader ipc (if in
                // processtree spawned by dglloader),
                // or get new values.

                int dlOpenAddr, dlSymAddr;
                if (oldUUID.length()) {
                    std::shared_ptr<DGLIPC> oldDGLIPC =
                            DGLIPC::CreateFromUUID(oldUUID);
                    oldDGLIPC->getDLInternceptPointers(dlOpenAddr, dlSymAddr);
                } else {
                    const char* baseAddr = reinterpret_cast<char*>(
                            reinterpret_cast<intptr_t>(&dlclose));
                    dlOpenAddr = reinterpret_cast<char*>(
                                         reinterpret_cast<intptr_t>(&dlopen)) -
                                 baseAddr;
                    dlSymAddr = reinterpret_cast<char*>(
                                        reinterpret_cast<intptr_t>(&dlsym)) -
                                baseAddr;
                }
                dglIPC->setDLInternceptPointers(dlOpenAddr, dlSymAddr);
            }
#endif
            process.do_execvp();
#endif
        } else {
            childHandle = process.getHandle();
        }


        // process is running, but is suspended (user thread not started, some DLLMain-s
        // may be run by now)

        //Inject DGLWrapper to our process & run
        inject.injectPostExec(*dglIPC, process.getHandle(), process.getMainThread());
    }
    catch (const std::exception& e) {

        static_assert(sizeof(char) == sizeof(ipcMessage->m_message[0]),
                      "Wrong IPC message element size");

        if (int osError = Os::getLastosError()) {
            snprintf((char*)ipcMessage->m_message, 1000, "%s: %s\n", e.what(),
                     Os::translateOsError(osError).c_str());
        } else {
            snprintf((char*)ipcMessage->m_message, 1000, "%s\n", e.what());
        }

        if (childHandle) {
// We were able to start child process, but furhter setup failed.
// Kill the child ASAP.
#ifdef _WIN32
            TerminateProcess(childHandle, EXIT_FAILURE);
#else
            kill(childHandle, SIGPIPE);
#endif
            childHandle = 0;
        }

        ipcMessage->m_ok = int8_t(false);
    }

    Os::info("%s", ipcMessage->m_message);

    std::string shmemName = Os::getEnv("dgl_loader_shmem");
    if (shmemName.length()) {
        boost::interprocess::shared_memory_object shobj(
                boost::interprocess::open_only, shmemName.c_str(),
                boost::interprocess::read_write);
        boost::interprocess::mapped_region region(
                shobj, boost::interprocess::read_write);
        std::memset(region.get_address(), 0, region.get_size());
        memcpy(region.get_address(), ipcMessage, sizeof(IPCMessage));
    }

    std::string semaphore = Os::getEnv("dgl_loader_semaphore");
    if (semaphore.length()) {
        boost::interprocess::named_semaphore sem(boost::interprocess::open_only,
                                                 semaphore.c_str());
        sem.post();
    }

    if (!ipcMessage->m_ok) {
        DGL_ASSERT(!childHandle);

        // failure - we were not able to setup properly exit code.
        // The failure reason is passed via IPC. Parrent process should check
        // IPC data to distinguish
        // loader failure vs. non-zero exit code from loader's child.
        return EXIT_FAILURE;
    } else {

// Wait for child process to exit. Return child process exit code.
#ifdef _WIN32
        DWORD exitCode;
        WaitForSingleObject(childHandle, INFINITE);
        GetExitCodeProcess(childHandle, &exitCode);
        return exitCode;
#else
        int exitCode;
        waitpid(childHandle, &exitCode, 0);
        return WEXITSTATUS(exitCode);
#endif
    }
}
