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

#ifdef _WIN32
    #include <windows.h>
    #include "CompleteInject/CompleteInject.h"
    #define snprintf _snprintf
#else
    #include <sys/types.h>
    #include <sys/wait.h>
#endif

#include "process.h"

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <cstdio>
#include <stdint.h>

#pragma warning(push)
#pragma warning(disable:4512)//'boost::program_options::options_description' : assignment operator could not be generated
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

#ifdef __ANDROID__
#include <dlfcn.h>
#endif

#pragma warning(disable:4503)

#ifdef _WIN32
bool isProcess64Bit(HANDLE hProcess) {
    //get IsWow64Process function
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = 
        fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    BOOL isWow;

    //check if windows is 64-bit 
    bool isWindows64bit = false;

#ifdef _WIN64
    //if we are a 64 bit app, windows is definitely 64 bit
    isWindows64bit = true;
#else
    //we are 32-bit application. Check if we are in Wow64 mode
    if (fnIsWow64Process && fnIsWow64Process(GetCurrentProcess(),&isWow) && isWow) {
        //if we are in Wow64 mode, windows is 64 bit
        isWindows64bit = true;
    }
#endif

    //check if process is 64 bit
    if (isWindows64bit && fnIsWow64Process && fnIsWow64Process(hProcess, &isWow) && !isWow) {
        //windows is 64 bit, process is not in Wow64 mode, so process is 64 bit. This implies usage of 64 bit wrapper
        return true;
    }
    return false;
}
#endif


struct IPCMessage {
    IPCMessage():m_ok(true) { m_message[0] = 0; };
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
        
    char fileName[MAX_PATH];
    if (!GetModuleFileName(NULL, fileName, MAX_PATH)) {
        return ret;
    }
    std::string tmp = fileName;
    size_t splitPoint=tmp.find_last_of("/\\");
    return tmp.substr(0, splitPoint) + "\\" + ret;
#elif defined(__ANDROID__)
    return "/data/local/tmp/libdglwrapper.so";
#else
    return "libdglwrapper.so";
#endif
}

namespace po = boost::program_options;
using namespace std;

int main(int argc, char** argv) {

    IPCMessage message;
    IPCMessage* ipcMessage = &message; 

    DGLProcess::native_process_handle_t childHandle = 0;

    try {

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("egl", "use egl mode")
            ("nowait", "do not wait for debugger to connect");

        desc.add_options()
            ("port", po::value< vector<string> >(), "Debugger TCP port number.");

        po::options_description mandatory("Mandatory options");
        mandatory.add_options()
            ("execute", po::value< vector<string> >()->composing(), "command to execute. Use -- top supply additional args.");        

        po::positional_options_description p;
        p.add("execute", -1);

        po::options_description all; 
        all.add(desc).add(mandatory);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
            options(all).positional(p).run(), vm);
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

        std::string executable = vm["execute"].as< vector<string> >()[0];
        std::vector<std::string> arguments;

        for (size_t i = 0; i < vm["execute"].as< vector<string> >().size(); i++) {
            arguments.push_back(vm["execute"].as< vector<string> >()[i]);
        }

        std::string wrapperPath = getWrapperPath();
        Os::info("Executable: %s\nWrapper: %s\n\n\n", executable.c_str(), wrapperPath.c_str());

        std::shared_ptr<DGLIPC> dglIPC = DGLIPC::Create();
        Os::setEnv("dgl_uuid", dglIPC->getUUID().c_str());

        if (vm.count("egl")) {
            dglIPC->setDebuggerMode(DGLIPC::DebuggerMode::EGL);
        }

        if (vm.count("nowait")) {
            dglIPC->setWaitForConnection(false);
        }

        if (vm.count("port")) {
            std::string portStr = vm["port"].as< vector<string> >()[0];
            if (portStr.find("unix:") == 0) {
                dglIPC->setDebuggerPort(DGLIPC::DebuggerPortType::UNIX, portStr.substr(strlen("unix:")));
            } else if (portStr.find("tcp:") == 0) {
                dglIPC->setDebuggerPort(DGLIPC::DebuggerPortType::TCP, portStr.substr(strlen("tcp:")));
            } else {
                dglIPC->setDebuggerPort(DGLIPC::DebuggerPortType::TCP, portStr);
            }
        }

        DGLProcess process(executable, arguments);

        if (process.getHandle() == 0) {
            //we are in forked out process
#ifdef _WIN32
            assert(!"forked out on windows");
#else
            Os::setEnv("LD_PRELOAD", wrapperPath.c_str());

#ifdef __ANDROID__
            {
                const char* baseAddr = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(&dlclose));
                if (!Os::getEnv("dlopen_addr").length()) {

                    int addr = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(&dlopen)) - baseAddr;
                    std::ostringstream str; str << addr;
                    Os::setEnv("dlopen_addr", str.str().c_str());

                }
                 if (!Os::getEnv("dlsym_addr").length()) {
                     int addr = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(&dlsym)) - baseAddr;
                     std::ostringstream str; str << addr;
                     Os::setEnv("dlsym_addr", str.str().c_str());
                 }
            }
#endif
            process.do_execvp();

#endif
        } else {
            childHandle = process.getHandle();
        }


#ifdef _WIN32        
#ifdef _WIN64
        if (!isProcess64Bit(process.getHandle())) {
            throw std::runtime_error("Incompatible loader version: used 64bit, but process is not 64bit");
        }
#else
        if (isProcess64Bit(process.getHandle())) {
            throw std::runtime_error("Incompatible loader version: used 32bit, but process is not 32bit");
        }
#endif

        //process is running, but is suspended (user thread not started, some DLLMain-s may be run by now)

        //inject thread with wrapper library loading code, run through DLLMain and InitializeThread() function

#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
        Inject(process.getHandle(), wrapperPath.c_str(), "LoaderThread");
        dglIPC->waitForRemoteThreadSemaphore();
#else
        HANDLE thread = Inject(process.getHandle(), wrapperPath.c_str(), "LoaderThread");
        //wait for loader thread to finish dll inject
        WaitForSingleObject(thread, INFINITE);
#endif

        //resume process - now user thread is running
        //whole OpenGL should be wrapped by now

        if (ResumeThread(process.getMainThread()) == -1) {
            throw std::runtime_error("Cannot resume process");
        }
#endif

    } catch (const std::exception& e) {

        static_assert(sizeof(char) == sizeof(ipcMessage->m_message[0]), "Wrong IPC message element size");

        if (int osError = Os::getLastosError()) {
            snprintf((char*)ipcMessage->m_message, 1000, "%s: %s\n", e.what(), Os::translateOsError(osError).c_str());
        } else {
            snprintf((char*)ipcMessage->m_message, 1000, "%s\n", e.what());
        }

        if (childHandle) {
            //We were able to start child process, but furhter setup failed.
            //Kill the child ASAP.
#ifdef _WIN32
            TerminateProcess(childHandle, EXIT_FAILURE);
#else
            kill(childHandle, SIGPIPE);
#endif
            childHandle = 0;
        }

        ipcMessage->m_ok = false;
    }

    Os::info("%s", ipcMessage->m_message);

    std::string shmemName = Os::getEnv("dgl_loader_shmem");
    if (shmemName.length()) {
        boost::interprocess::shared_memory_object shobj(boost::interprocess::open_only, shmemName.c_str(), boost::interprocess::read_write);
        boost::interprocess::mapped_region region(shobj, boost::interprocess::read_write);
        std::memset(region.get_address(), 0, region.get_size());
        memcpy(region.get_address(), ipcMessage, sizeof(IPCMessage));
    }

    std::string semaphore = Os::getEnv("dgl_loader_semaphore");
    if (semaphore.length()) {
        boost::interprocess::named_semaphore sem(boost::interprocess::open_only, semaphore.c_str());
        sem.post();
    }

    if (!ipcMessage->m_ok) {
        assert(!childHandle);

        //failure - we were not able to setup properly exit code. 
        //The failure reason is passed via IPC. Parrent process should check IPC data to distinguish
        //loader failure vs. non-zero exit code from loader's child.
        return EXIT_FAILURE;
    } else {
    
        //Wait for child process to exit. Return child process exit code.
#ifdef _WIN32
        DWORD exitCode;
        WaitForSingleObject( childHandle, INFINITE );
        GetExitCodeProcess( childHandle, &exitCode);
        return exitCode;
#else
        int exitCode;
        waitpid(childHandle, &exitCode, 0);
        return WEXITSTATUS(exitCode);
#endif
    }
}
