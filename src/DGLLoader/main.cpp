
#pragma warning(disable : 4996) 

#include <windows.h>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <stdint.h>


#include "CompleteInject/CompleteInject.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>

#include <boost/program_options.hpp>

#include "DGLCommon/os.h"

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


struct IPCMessage {
    IPCMessage(uint32_t s):status(s) { message[0] = 0; };
    uint32_t status;
    char message[1000];
};


std::string getWrapperPath() {

#ifdef _WIN64
    std::string ret = "DGLWrapper64\\OpenGL32.dll";
#else
    std::string ret = "DGLWrapper\\OpenGL32.dll";
#endif
        
    char fileName[MAX_PATH];
    if (!GetModuleFileName(NULL, fileName, MAX_PATH)) {
        return ret;
    }
    std::string tmp = fileName;
    size_t splitPoint=tmp.find_last_of("/\\");
    return tmp.substr(0, splitPoint) + "\\" + ret;
}

namespace po = boost::program_options;
using namespace std;

int main(int argc, char** argv) {

    IPCMessage default(EXIT_SUCCESS);
    IPCMessage* ipcMessage = &default;    

    try {

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("egl", "use egl mode");

        po::options_description mandatory("Mandatory options");
        mandatory.add_options()
            ("execute", po::value< vector<string> >()->composing(), "command to execute");        

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

        }

        if (!vm.count("execute")) {
            std::ostringstream out;
            out << "Nothing to execute.\n" << desc << "\n" << mandatory << "\n";
            throw std::runtime_error(out.str());
        }

        const char* executable = vm["execute"].as< vector<string> >()[0].c_str();
        std::string arguments = executable; 
        for (size_t i = 1; i < vm["execute"].as< vector<string> >().size(); i++) {
            arguments += " ";
            arguments += vm["execute"].as< vector<string> >()[i];
        }

        char path[MAX_PATH];
        if (!GetCurrentDirectory(MAX_PATH, path)) {
            throw std::runtime_error("GetCurrentDirectory failed");
        }

        std::string wrapperPath = getWrapperPath();

        printf("Executable: %s\nPath: %s\nArguments: %s\nWrapper: %s\n\n\n", executable, path, arguments.c_str(), wrapperPath.c_str());

        //prepare some structures for CreateProcess output
        STARTUPINFOA startupInfo;
        memset(&startupInfo, 0, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        PROCESS_INFORMATION processInformation; 
        memset(&processInformation, 0, sizeof(processInformation));

        //try run process (suspended - will not run user thread)

        if (CreateProcessA(
            executable,
            (LPSTR)arguments.c_str(),
            NULL, 
            NULL,
            FALSE, 
            CREATE_SUSPENDED,
            NULL,
            path,
            &startupInfo, 
            &processInformation) == 0 ) {

                throw std::runtime_error("Cannot create process");
        }

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

        HANDLE thread = Inject(processInformation.hProcess, wrapperPath.c_str(), "InitializeThread");

        //wait for remote thread to end - wait for library to load and InitializeThread() to return

        WaitForSingleObject(thread, INFINITE); 


        //resume process - now user thread is running
        //whole OpenGL should be wrapped by now

        if (ResumeThread(processInformation.hThread) == -1) {
            throw std::runtime_error("Cannot resume process");
        }
    } catch (const std::exception& e) {

        ipcMessage->status = EXIT_FAILURE;

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
    }


    printf(ipcMessage->message);


    std::string shmemName = Os::getEnv("dgl_loader_shmem");
    if (shmemName.length()) {
        boost::interprocess::shared_memory_object shobj(boost::interprocess::open_only, shmemName.c_str(), boost::interprocess::read_write);
        boost::interprocess::mapped_region region(shobj, boost::interprocess::read_write);
        std::memset(region.get_address(), 0, region.get_size());
        memcpy(region.get_address(), ipcMessage, sizeof(IPCMessage));
    }

    std::string semaphore = Os::getEnv("dgl_loader_semaphore");
    if (semaphore.length()) {

        //this is a rather dirty WA for local debugging
        //we fire given semaphore, when we are ready for connection (now!)

        boost::interprocess::named_semaphore sem(boost::interprocess::open_only, semaphore.c_str());
        sem.post();

    }

    return ipcMessage->status;
}