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
#else
    //TODO
#endif

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <stdint.h>



#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp> 
#include <boost/program_options.hpp>

#include "DGLCommon/os.h"
#include <DGLCommon/wa.h>

#ifdef __ANDROID__
#include <dlfcn.h>
#endif


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
    IPCMessage(uint32_t s):status(s) { message[0] = 0; };
    uint32_t status;
    char message[1000];
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
#elif __ANDROID__
    return "/data/local/tmp/libdglwrapper.so";
#else
    return "libdglwrapper.so";
#endif
}

std::string getCurrentDirectory() { 
#ifndef _WIN32
    #define MAX_PATH PATH_MAX
#endif
     char path[MAX_PATH];

#ifdef _WIN32
     if (!GetCurrentDirectory(MAX_PATH, path)) {
#else
     if (!getcwd(path, MAX_PATH)) {
#endif
        throw std::runtime_error("GetCurrentDirectory failed");
     }
     return path;
}

namespace po = boost::program_options;
using namespace std;

int main(int argc, char** argv) {

    IPCMessage message(EXIT_SUCCESS);
    IPCMessage* ipcMessage = &message;    

    try {

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("egl", "use egl mode");

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

        if (vm.count("egl")) {
            Os::setEnv("dgl_mode", "egl");
        }

        if (!vm.count("execute")) {
            std::ostringstream out;
            out << "Nothing to execute.\n" << desc << "\n" << mandatory << "\n";
            throw std::runtime_error(out.str());
        }

        std::string executable = vm["execute"].as< vector<string> >()[0];
        std::vector<std::string> arguments;
        std::string argumentString;
        for (size_t i = 0; i < vm["execute"].as< vector<string> >().size(); i++) {
            arguments.push_back(vm["execute"].as< vector<string> >()[i]);
            argumentString += (i > 0?" ":"") + arguments[i];
        }

        std::string wrapperPath = getWrapperPath();
        std::string path = getCurrentDirectory();

        printf("Executable: %s\nPath: %s\nArguments: %s\nWrapper: %s\n\n\n", executable.c_str(), path.c_str(), argumentString.c_str(), wrapperPath.c_str());

#ifdef  WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
        //Workaround fo ARM Mali OpenGL ES wrapper on Windows: see LoaderThread() for description
        //as we do not return from LoaderThread() immediately, we need a semaphore to know when to resume application
        std::ostringstream remoteThreadSemaphoreStr;
        remoteThreadSemaphoreStr << boost::uuids::random_generator()();
        Os::setEnv("dgl_remote_thread_semaphore", remoteThreadSemaphoreStr.str().c_str());
        boost::interprocess::named_semaphore remoteThreadSemaphore(boost::interprocess::open_or_create, remoteThreadSemaphoreStr.str().c_str(), 0);
#endif


#ifdef _WIN32
        //prepare some structures for CreateProcess output
        STARTUPINFOA startupInfo;
        memset(&startupInfo, 0, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        PROCESS_INFORMATION processInformation; 
        memset(&processInformation, 0, sizeof(processInformation));

        //try run process (suspended - will not run user thread)

        if (CreateProcessA(
            executable.c_str(),
            (LPSTR)argumentString.c_str(),
            NULL, 
            NULL,
            FALSE, 
            CREATE_SUSPENDED,
            NULL,
            path.c_str(),
            &startupInfo, 
            &processInformation) == 0 ) {

                throw std::runtime_error("Cannot create process");
        }
#else
        pid_t pid = fork();
        if (pid == -1) {
            throw std::runtime_error("Cannot fork process");
        }
        if (pid == 0) {
            std::vector<std::vector<char> > argvs(arguments.size()); 
            std::vector<char*> argv(arguments.size());
            for (size_t i = 0; i < arguments.size(); i++) {
                std::copy(arguments[i].begin(), arguments[i].end(), std::back_inserter<std::vector<char> >(argvs[i]));
                argvs[i].push_back('\0');
                argv[i] = &argvs[i][0];
            }
            argv.push_back(NULL);

            Os::setEnv("LD_PRELOAD", wrapperPath.c_str());

#ifdef __ANDROID__
           {
               int addr = reinterpret_cast<char*>(&dlopen) - reinterpret_cast<char*>(&dlclose);
               std::ostringstream str; str << addr;
               Os::setEnv("dlopen_addr", str.str().c_str());
           }
           {
               int addr = reinterpret_cast<char*>(&dlsym) - reinterpret_cast<char*>(&dlclose);
               std::ostringstream str; str << addr;
               Os::setEnv("dlsym_addr", str.str().c_str());
           }
#endif

            execvp(executable.c_str(), &argv[0]);
            throw std::runtime_error("Cannot execute process");
        }
#endif

#ifdef _WIN32        
#ifdef _WIN64
        if (!isProcess64Bit(processInformation.hProcess)) {
            throw std::runtime_error("Incompatible loader version: used 64bit, but process is not 64bit");
        }
#else
        if (isProcess64Bit(processInformation.hProcess)) {
            throw std::runtime_error("Incompatible loader version: used 32bit, but process is not 32bit");
        }
#endif

        //process is running, but is suspended (user thread not started, some DLLMain-s may be run by now)

        //inject thread with wrapper library loading code, run through DLLMain and InitializeThread() function

        HANDLE thread = Inject(processInformation.hProcess, wrapperPath.c_str(), "LoaderThread");

        //wait for loader thread to finish dll inject
#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
        remoteThreadSemaphore.wait();
#else
        WaitForSingleObject(thread, INFINITE);
#endif

        //resume process - now user thread is running
        //whole OpenGL should be wrapped by now

        if (ResumeThread(processInformation.hThread) == -1) {
            throw std::runtime_error("Cannot resume process");
        }
#endif

    } catch (const std::exception& e) {

        ipcMessage->status = EXIT_FAILURE;

#ifdef _WIN32        
        //generic, GetLastError() aware error printer

        if ( DWORD lastError = GetLastError()) {

            char* errorText;
            FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM
                |FORMAT_MESSAGE_ALLOCATE_BUFFER
                |FORMAT_MESSAGE_IGNORE_INSERTS,  
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&errorText,
                0,
                NULL);

            _snprintf(ipcMessage->message, 1000, "%s: %s\n", e.what(), errorText);

            LocalFree(errorText);

        } else {
            _snprintf(ipcMessage->message, 1000, "%s\n", e.what());
        }
#else
        if (errno) {
            snprintf(ipcMessage->message, 1000, "%s: %s\n", e.what(), strerror(errno));
        } else {
            snprintf(ipcMessage->message, 1000, "%s\n", e.what());
        }
#endif
    }

    printf("%s", ipcMessage->message);

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

    return ipcMessage->status;
}
