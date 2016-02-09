#include <windows.h>
#include <stdio.h>

/***************************************************************************************************/
//	Function: 
//		Inject
//	
//	Parameters:
//		HANDLE hProcess - The handle to the process to inject the DLL into.
//
//		const char* dllname - The name of the DLL to inject into the process.
//		
//		const char* funcname - The name of the function to call once the DLL has been injected.
//
//	Description:
//		This function will inject a DLL into a process and execute an exported function
//		from the DLL to "initialize" it. The function should be in the format shown below,
//		not parameters and no return type. Do not forget to prefix extern "C" if you are in C++
//
//			__declspec(dllexport) void FunctionName(void)
//
//		The function that is called in the injected DLL
//		-MUST- return, the loader waits for the thread to terminate before removing the 
//		allocated space and returning control to the Loader. This method of DLL injection
//		also adds error handling, so the end user knows if something went wrong.
/***************************************************************************************************/

HANDLE Inject(HANDLE hProcess, const char* dllname, const char* funcname)
{
//------------------------------------------//
// Function variables.						//
//------------------------------------------//

	// Main DLL we will need to load
	HMODULE kernel32	= NULL;

	// Main functions we will need to import
	FARPROC loadlibrary		= NULL;
	FARPROC getprocaddress	= NULL;
	FARPROC exitprocess		= NULL;
	FARPROC exitthread		= NULL;
	FARPROC freelibraryandexitthread = NULL;

	// The workspace we will build the codecave on locally
	LPBYTE workspace		= NULL;

#ifdef _WIN64
	DWORD64 workspaceIndex	= 0;
#else
    DWORD workspaceIndex	= 0;
#endif

	// The memory in the process we write to
	LPBYTE codecaveAddress	= NULL;

	// Strings we have to write into the process
	char injectDllName[MAX_PATH + 1]	= {0};
	char injectFuncName[MAX_PATH + 1]	= {0};
	char injectError0[MAX_PATH + 1]		= {0};
	char injectError1[MAX_PATH + 1]		= {0};
	char injectError2[MAX_PATH + 1]		= {0};
	char user32Name[MAX_PATH + 1]		= {0};
	char msgboxName[MAX_PATH + 1]		= {0};

	// Placeholder addresses to use the strings
    LPVOID user32NameAddr	= NULL;
    LPVOID user32Addr		= NULL;
    LPVOID msgboxNameAddr	= NULL;
    LPVOID msgboxAddr		= NULL;
    //LPVOID dllAddr			= NULL;
    LPVOID dllNameAddr		= NULL;
    LPVOID funcNameAddr		= NULL;
    LPVOID error0Addr		= NULL;
    LPVOID error1Addr		= NULL;
    LPVOID error2Addr		= NULL;

    // Where the codecave execution should begin at
	LPVOID codecaveExecAddr = 0;

	// Temp variables
#ifdef _WIN64
	DWORD64 dwTmpSize = 0;
#else
    DWORD dwTmpSize = 0;
#endif

	// Old protection on page we are writing to in the process and the bytes written
	DWORD oldProtect	= 0;	
	SIZE_T bytesRet		= 0;

//------------------------------------------//
// Variable initialization.					//
//------------------------------------------//

	// Get the address of the main DLL
	kernel32	= LoadLibraryA("kernel32.dll");

	// Get our functions
	loadlibrary		= GetProcAddress(kernel32,	"LoadLibraryA");
	getprocaddress	= GetProcAddress(kernel32,	"GetProcAddress");
	exitprocess		= GetProcAddress(kernel32,	"ExitProcess");
	exitthread		= GetProcAddress(kernel32,	"ExitThread");
	freelibraryandexitthread = GetProcAddress(kernel32,	"FreeLibraryAndExitThread");

// This section will cause compiler warnings on VS8, 
// you can upgrade the functions or ignore them

	// Build names
	_snprintf_s(injectDllName, MAX_PATH, _TRUNCATE, "%s", dllname);
	_snprintf_s(injectFuncName, MAX_PATH, _TRUNCATE, "%s", funcname);
	_snprintf_s(user32Name, MAX_PATH, _TRUNCATE, "user32.dll");
	_snprintf_s(msgboxName, MAX_PATH, _TRUNCATE, "MessageBoxA");

	// Build error messages
    _snprintf_s(injectError0, MAX_PATH, _TRUNCATE, "Error");
    _snprintf_s(injectError1, MAX_PATH, _TRUNCATE, "Could not load the dll: %s", injectDllName);
    _snprintf_s(injectError2, MAX_PATH, _TRUNCATE, "Could not load the function: %s", injectFuncName);

	// Create the workspace
	workspace = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);

	// Allocate space for the codecave in the process
	codecaveAddress = (LPBYTE) VirtualAllocEx(hProcess, 0, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

// Note there is no error checking done above for any functions that return a pointer/handle.
// I could have added them, but it'd just add more messiness to the code and not provide any real
// benefit. It's up to you though in your final code if you want it there or not.

//------------------------------------------//
// Data and string writing.					//
//------------------------------------------//

	// Write out the address for the user32 dll address
	user32Addr = workspaceIndex + codecaveAddress;
	dwTmpSize = 0;
	memcpy(workspace + workspaceIndex, &dwTmpSize, sizeof(dwTmpSize));
	workspaceIndex += sizeof(dwTmpSize);

	// Write out the address for the MessageBoxA address
	msgboxAddr = workspaceIndex + codecaveAddress;
	dwTmpSize = 0;
	memcpy(workspace + workspaceIndex, &dwTmpSize, sizeof(dwTmpSize));
	workspaceIndex += sizeof(dwTmpSize);

	//// Write out the address for the injected DLL's module
	//dllAddr = workspaceIndex + codecaveAddress;
	//dwTmpSize = 0;
	//memcpy(workspace + workspaceIndex, &dwTmpSize, sizeof(dwTmpSize));
	//workspaceIndex += 4;

	// User32 Dll Name
	user32NameAddr = workspaceIndex + codecaveAddress;
	dwTmpSize = (DWORD)strlen(user32Name) + 1;
	memcpy(workspace + workspaceIndex, user32Name, dwTmpSize);
	workspaceIndex += dwTmpSize;

	// MessageBoxA name
	msgboxNameAddr = workspaceIndex + codecaveAddress;
	dwTmpSize = (DWORD)strlen(msgboxName) + 1;
	memcpy(workspace + workspaceIndex, msgboxName, dwTmpSize);
	workspaceIndex += dwTmpSize;

	// Dll Name
	dllNameAddr = workspaceIndex + codecaveAddress;
	dwTmpSize = (DWORD)strlen(injectDllName) + 1;
	memcpy(workspace + workspaceIndex, injectDllName, dwTmpSize);
	workspaceIndex += dwTmpSize;

	// Function Name
	funcNameAddr = workspaceIndex + codecaveAddress;
	dwTmpSize = (DWORD)strlen(injectFuncName) + 1;
	memcpy(workspace + workspaceIndex, injectFuncName, dwTmpSize);
	workspaceIndex += dwTmpSize;

	// Error Message 1
	error0Addr = workspaceIndex + codecaveAddress;
	dwTmpSize = (DWORD)strlen(injectError0) + 1;
	memcpy(workspace + workspaceIndex, injectError0, dwTmpSize);
	workspaceIndex += dwTmpSize;

	// Error Message 2
	error1Addr = workspaceIndex + codecaveAddress;
	dwTmpSize = (DWORD)strlen(injectError1) + 1;
	memcpy(workspace + workspaceIndex, injectError1, dwTmpSize);
	workspaceIndex += dwTmpSize;

	// Error Message 3
	error2Addr = workspaceIndex + codecaveAddress;
	dwTmpSize = (DWORD)strlen(injectError2) + 1;
	memcpy(workspace + workspaceIndex, injectError2, dwTmpSize);
	workspaceIndex += dwTmpSize;

	// Pad a few INT3s after string data is written for seperation
	workspace[workspaceIndex++] = 0xCC;
	workspace[workspaceIndex++] = 0xCC;
	workspace[workspaceIndex++] = 0xCC;

	// Store where the codecave execution should begin
	codecaveExecAddr = workspaceIndex + codecaveAddress;

// For debugging - infinite loop, attach onto process and step over
	//workspace[workspaceIndex++] = 0xEB;
	//workspace[workspaceIndex++] = 0xFE;

//------------------------------------------//
// User32.dll loading.						//
//------------------------------------------//

// User32 DLL Loading
#ifdef _WIN64
	// SUB RSP, 0x28 - x64 ABI requires us to alloc shadow space (-0x20 here) and align stack to 16 (-0x08 here)
	workspace[workspaceIndex++] = 0x48;
	workspace[workspaceIndex++] = 0x83;
	workspace[workspaceIndex++] = 0xEC;
	workspace[workspaceIndex++] = 0x28;
	
    // MOV RCX, ADDRESS("user32.dll") - Move the address of the DLL name to use in LoadLibraryA
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xB9;
    memcpy(workspace + workspaceIndex, &user32NameAddr, sizeof(user32NameAddr));
    workspaceIndex += sizeof(user32NameAddr);

    // MOV RAX, ADDRESS(LoadLibraryA) - Move the address of LoadLibraryA into RAX
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xB8;
    memcpy(workspace + workspaceIndex, &loadlibrary, sizeof(loadlibrary));
    workspaceIndex += sizeof(loadlibrary);

    // CALL RAX - Call LoadLibraryA
    workspace[workspaceIndex++] = 0xFF;
    workspace[workspaceIndex++] = 0xD0;

#else
	// PUSH 0x00000000 - Push the address of the DLL name to use in LoadLibraryA
	workspace[workspaceIndex++] = 0x68;
	memcpy(workspace + workspaceIndex, &user32NameAddr, 4);
	workspaceIndex += 4;

	// MOV EAX, ADDRESS - Move the address of LoadLibraryA into EAX
	workspace[workspaceIndex++] = 0xB8;
	memcpy(workspace + workspaceIndex, &loadlibrary, 4);
	workspaceIndex += 4;

	// CALL EAX - Call LoadLibraryA
	workspace[workspaceIndex++] = 0xFF;
	workspace[workspaceIndex++] = 0xD0;
#endif

// MessageBoxA Loading
#ifdef _WIN64
    //MOV RCX, RAX - Module to use in GetProcAddress (user32.dll)
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0x89;
    workspace[workspaceIndex++] = 0xC1;

    //MOV RDX, ADDRESS("MessageBoxA") - Address of the function name to load ("MessageBoxA")
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xBA;
    memcpy(workspace + workspaceIndex, &msgboxNameAddr, sizeof(msgboxNameAddr));
    workspaceIndex += sizeof(msgboxNameAddr);

    //MOV RAX, ADDRESS(GetProcAddress) -  Move the address of GetProcAddress into RAX
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xB8;
    memcpy(workspace + workspaceIndex, &getprocaddress, sizeof(getprocaddress));
    workspaceIndex += sizeof(getprocaddress);

    // CALL RAX - Call GetProcAddress
    workspace[workspaceIndex++] = 0xFF;
    workspace[workspaceIndex++] = 0xD0;

    // MOV [ADDRESS], RAX Save the address to our variable
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xA3;
    memcpy(workspace + workspaceIndex, &msgboxAddr, sizeof(msgboxAddr));
    workspaceIndex += sizeof(msgboxAddr);
#else
	// PUSH 0x000000 - Push the address of the function name to load
	workspace[workspaceIndex++] = 0x68;
	memcpy(workspace + workspaceIndex, &msgboxNameAddr, 4);
	workspaceIndex += 4;

	// Push EAX, module to use in GetProcAddress
	workspace[workspaceIndex++] = 0x50;

	// MOV EAX, ADDRESS - Move the address of GetProcAddress into EAX
	workspace[workspaceIndex++] = 0xB8;
	memcpy(workspace + workspaceIndex, &getprocaddress, 4);
	workspaceIndex += 4;

	// CALL EAX - Call GetProcAddress
	workspace[workspaceIndex++] = 0xFF;
	workspace[workspaceIndex++] = 0xD0;

	// MOV [ADDRESS], EAX - Save the address to our variable
	workspace[workspaceIndex++] = 0xA3;
	memcpy(workspace + workspaceIndex, &msgboxAddr, 4);
	workspaceIndex += 4;
#endif

//------------------------------------------//
// Injected dll loading.					//
//------------------------------------------//

/*
	// This is the way the following assembly code would look like in C/C++

	// Load the injected DLL into this process
	HMODULE h = LoadLibrary("mydll.dll");
	if(!h)
	{
		MessageBox(0, "Could not load the dll: mydll.dll", "Error", MB_ICONERROR);
		ExitProcess(0);
	}

	// Get the address of the export function
	FARPROC p = GetProcAddress(h, "Initialize");
	if(!p)
	{
		MessageBox(0, "Could not load the function: Initialize", "Error", MB_ICONERROR);
		ExitProcess(0);
	}

	// So we do not need a function pointer interface
	__asm call p

	// Exit the thread so the loader continues
	ExitThread(0);
*/

// DLL Loading
#ifdef _WIN64

    // MOV RCX, ADRRESS("own.dll") - address of the DLL name to use in LoadLibraryA (own dll)
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xB9;
    memcpy(workspace + workspaceIndex, &dllNameAddr, sizeof(dllNameAddr));
    workspaceIndex += sizeof(dllNameAddr);

    // MOV RAX, ADDRESS(LoadLibraryA) - Move the address of LoadLibraryA into RAX
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xB8;
    memcpy(workspace + workspaceIndex, &loadlibrary, sizeof(loadlibrary));
    workspaceIndex += sizeof(loadlibrary);

    // CALL RAX - Call LoadLibraryA
    workspace[workspaceIndex++] = 0xFF;
    workspace[workspaceIndex++] = 0xD0;

#else
	// PUSH 0x00000000 - Push the address of the DLL name to use in LoadLibraryA
	workspace[workspaceIndex++] = 0x68;
	memcpy(workspace + workspaceIndex, &dllNameAddr, 4);
	workspaceIndex += 4;

	// MOV EAX, ADDRESS - Move the address of LoadLibraryA into EAX
	workspace[workspaceIndex++] = 0xB8;
	memcpy(workspace + workspaceIndex, &loadlibrary, 4);
	workspaceIndex += 4;

	// CALL EAX - Call LoadLibraryA
	workspace[workspaceIndex++] = 0xFF;
	workspace[workspaceIndex++] = 0xD0;
#endif

// Error Checking
#ifdef _WIN64
    //CMP RAX, 0
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0x83;
    workspace[workspaceIndex++] = 0xF8;
    workspace[workspaceIndex++] = 0x00;

    //JNZ EIP + 0x39 to skip over error code
    workspace[workspaceIndex++] = 0x75;
    workspace[workspaceIndex++] = 0x39;
#else
	// CMP EAX, 0
	workspace[workspaceIndex++] = 0x83;
	workspace[workspaceIndex++] = 0xF8;
	workspace[workspaceIndex++] = 0x00;

// JNZ EIP + 0x1E to skip over error code
	workspace[workspaceIndex++] = 0x75;
	workspace[workspaceIndex++] = 0x1E;
#endif


// Error Code 1
#ifdef _WIN64
    //XOR RCX, RCX
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0x31;
    workspace[workspaceIndex++] = 0xC9;

    //MOV RDX, ADDRESS(" Push the address of the MessageBox title")
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xBA;
    memcpy(workspace + workspaceIndex, &error0Addr, sizeof(error0Addr));
    workspaceIndex += sizeof(error0Addr);

    //MOV R8, ADDRESS("MessageBox message")
    workspace[workspaceIndex++] = 0x49;
    workspace[workspaceIndex++] = 0xB8;
    memcpy(workspace + workspaceIndex, &error1Addr, sizeof(error1Addr));
    workspaceIndex += sizeof(error1Addr);

    //MOV R9, 0x10
    workspace[workspaceIndex++] = 0x49;
    workspace[workspaceIndex++] = 0xC7;
    workspace[workspaceIndex++] = 0xC1;
    workspace[workspaceIndex++] = 0x10;
    workspace[workspaceIndex++] = 0x00;
    workspace[workspaceIndex++] = 0x00;
    workspace[workspaceIndex++] = 0x00;

    //MOX RAX, ADDRES("MessageBoxA")
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xA1;
    memcpy(workspace + workspaceIndex, &msgboxAddr, sizeof(msgboxAddr));
    workspaceIndex += sizeof(msgboxAddr);

    //CALL RAX
    workspace[workspaceIndex++] = 0xFF;
    workspace[workspaceIndex++] = 0xD0;
#else
	// MessageBox
		// PUSH 0x10 (MB_ICONHAND)
		workspace[workspaceIndex++] = 0x6A;
		workspace[workspaceIndex++] = 0x10;

		// PUSH 0x000000 - Push the address of the MessageBox title
		workspace[workspaceIndex++] = 0x68;
		memcpy(workspace + workspaceIndex, &error0Addr, 4);
		workspaceIndex += 4;

		// PUSH 0x000000 - Push the address of the MessageBox message
		workspace[workspaceIndex++] = 0x68;
		memcpy(workspace + workspaceIndex, &error1Addr, 4);
		workspaceIndex += 4;

		// Push 0
		workspace[workspaceIndex++] = 0x6A;
		workspace[workspaceIndex++] = 0x00;

		// MOV EAX, [ADDRESS] - Move the address of MessageBoxA into EAX
		workspace[workspaceIndex++] = 0xA1;
		memcpy(workspace + workspaceIndex, &msgboxAddr, 4);
		workspaceIndex += 4;

		// CALL EAX - Call MessageBoxA
		workspace[workspaceIndex++] = 0xFF;
		workspace[workspaceIndex++] = 0xD0;
#endif

	// ExitProcess
#ifdef _WIN64
        //XOR RCX, RCX
        workspace[workspaceIndex++] = 0x48;
        workspace[workspaceIndex++] = 0x31;
        workspace[workspaceIndex++] = 0xC9;

        //MOV RAX, ADDRESS(ExitProcess)
        workspace[workspaceIndex++] = 0x48;
        workspace[workspaceIndex++] = 0xB8;
        memcpy(workspace + workspaceIndex, &exitprocess, sizeof(exitprocess));
        workspaceIndex += sizeof(exitprocess);

        //CALL RAX
        workspace[workspaceIndex++] = 0xFF;
        workspace[workspaceIndex++] = 0xD0;

#else
		// Push 0
		workspace[workspaceIndex++] = 0x6A;
		workspace[workspaceIndex++] = 0x00;

		// MOV EAX, ADDRESS - Move the address of ExitProcess into EAX
		workspace[workspaceIndex++] = 0xB8;
		memcpy(workspace + workspaceIndex, &exitprocess, 4);
		workspaceIndex += 4;

		// CALL EAX - Call MessageBoxA
		workspace[workspaceIndex++] = 0xFF;
		workspace[workspaceIndex++] = 0xD0;
#endif

////	Now we have the address of the injected DLL, so save the handle
//#ifdef _WIN64
//    workspace[workspaceIndex++] = 0x48;
//    workspace[workspaceIndex++] = 0xA3;
//    memcpy(workspace + workspaceIndex, &dllAddr, sizeof(dllAddr));
//    workspaceIndex += sizeof(dllAddr);
//#else
//	// MOV [ADDRESS], EAX - Save the address to our variable
//	workspace[workspaceIndex++] = 0xA3;
//	memcpy(workspace + workspaceIndex, &dllAddr, 4);
//	workspaceIndex += 4;
//#endif

// Load the initilize function from it
#ifdef _WIN64
    //MOV RCX, RAX
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0x89;
    workspace[workspaceIndex++] = 0xC1;

    //MOV RDX, ADDRESS(funcName)
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xBA;
    memcpy(workspace + workspaceIndex, &funcNameAddr, sizeof(funcNameAddr));
    workspaceIndex += sizeof(funcNameAddr);

    // MOV RAX, ADDRESS(GetProcAddress) - Move the address of GetProcAddress into RAX
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xB8;
    memcpy(workspace + workspaceIndex, &getprocaddress, sizeof(getprocaddress));
    workspaceIndex += sizeof(getprocaddress);

    // CALL RAX - Call GetProcAddress
    workspace[workspaceIndex++] = 0xFF;
    workspace[workspaceIndex++] = 0xD0;
#else
	// PUSH 0x000000 - Push the address of the function name to load
	workspace[workspaceIndex++] = 0x68;
	memcpy(workspace + workspaceIndex, &funcNameAddr, 4);
	workspaceIndex += 4;

	// Push EAX, module to use in GetProcAddress
	workspace[workspaceIndex++] = 0x50;

	// MOV EAX, ADDRESS - Move the address of GetProcAddress into EAX
	workspace[workspaceIndex++] = 0xB8;
	memcpy(workspace + workspaceIndex, &getprocaddress, 4);
	workspaceIndex += 4;

	// CALL EAX - Call GetProcAddress
	workspace[workspaceIndex++] = 0xFF;
	workspace[workspaceIndex++] = 0xD0;
#endif

// Error Checking
#ifdef _WIN64
    // CMP EAX, 0
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0x83;
    workspace[workspaceIndex++] = 0xF8;
    workspace[workspaceIndex++] = 0x00;

    // JNZ EIP + 0x37 to skip over error code
    workspace[workspaceIndex++] = 0x75;
    workspace[workspaceIndex++] = 0x37;
#else
	// CMP EAX, 0
	workspace[workspaceIndex++] = 0x83;
	workspace[workspaceIndex++] = 0xF8;
	workspace[workspaceIndex++] = 0x00;

// JNZ EIP + 0x1C to skip eror code
	workspace[workspaceIndex++] = 0x75;
	workspace[workspaceIndex++] = 0x1C;
#endif

// Error Code 2
	// MessageBox
#ifdef _WIN64
    //XOR RCX, RCX
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0x31;
    workspace[workspaceIndex++] = 0xC9;

    //MOV RDX, ADDRESS(" Push the address of the MessageBox title")
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xBA;
    memcpy(workspace + workspaceIndex, &error0Addr, sizeof(error0Addr));
    workspaceIndex += sizeof(error0Addr);

    //MOV R8, ADDRESS("MessageBox message")
    workspace[workspaceIndex++] = 0x49;
    workspace[workspaceIndex++] = 0xB8;
    memcpy(workspace + workspaceIndex, &error2Addr, sizeof(error2Addr));
    workspaceIndex += sizeof(error1Addr);

    //MOV R9, 0x10
    workspace[workspaceIndex++] = 0x49;
    workspace[workspaceIndex++] = 0xC7;
    workspace[workspaceIndex++] = 0xC1;
    workspace[workspaceIndex++] = 0x10;
    workspace[workspaceIndex++] = 0x00;
    workspace[workspaceIndex++] = 0x00;
    workspace[workspaceIndex++] = 0x00;

    //MOX RAX, ADDRES("MessageBoxA")
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xA1;
    memcpy(workspace + workspaceIndex, &msgboxAddr, sizeof(msgboxAddr));
    workspaceIndex += sizeof(msgboxAddr);

    //CALL RAX
    workspace[workspaceIndex++] = 0xFF;
    workspace[workspaceIndex++] = 0xD0;
#else
		// PUSH 0x10 (MB_ICONHAND)
		workspace[workspaceIndex++] = 0x6A;
		workspace[workspaceIndex++] = 0x10;

		// PUSH 0x000000 - Push the address of the MessageBox title
		workspace[workspaceIndex++] = 0x68;
		memcpy(workspace + workspaceIndex, &error0Addr, 4);
		workspaceIndex += 4;

		// PUSH 0x000000 - Push the address of the MessageBox message
		workspace[workspaceIndex++] = 0x68;
		memcpy(workspace + workspaceIndex, &error2Addr, 4);
		workspaceIndex += 4;

		// Push 0
		workspace[workspaceIndex++] = 0x6A;
		workspace[workspaceIndex++] = 0x00;

		// MOV EAX, ADDRESS - Move the address of MessageBoxA into EAX
		workspace[workspaceIndex++] = 0xA1;
		memcpy(workspace + workspaceIndex, &msgboxAddr, 4);
		workspaceIndex += 4;

		// CALL EAX - Call MessageBoxA
		workspace[workspaceIndex++] = 0xFF;
		workspace[workspaceIndex++] = 0xD0;
#endif

	// ExitProcess
#ifdef _WIN64
        //XOR RCX, RCX
        workspace[workspaceIndex++] = 0x48;
        workspace[workspaceIndex++] = 0x31;
        workspace[workspaceIndex++] = 0xC9;

        //MOV RAX, ADDRESS(ExitProcess)
        workspace[workspaceIndex++] = 0x48;
        workspace[workspaceIndex++] = 0xB8;
        memcpy(workspace + workspaceIndex, &exitprocess, sizeof(exitprocess));
        workspaceIndex += sizeof(exitprocess);

#else
		// Push 0
		workspace[workspaceIndex++] = 0x6A;
		workspace[workspaceIndex++] = 0x00;

		// MOV EAX, ADDRESS - Move the address of ExitProcess into EAX
		workspace[workspaceIndex++] = 0xB8;
		memcpy(workspace + workspaceIndex, &exitprocess, 4);
		workspaceIndex += 4;
#endif

//	Now that we have the address of the function, we cam call it, 
// if there was an error, the messagebox would be called as well.

	// CALL EAX (RAX) - Call ExitProcess -or- the Initialize function
	workspace[workspaceIndex++] = 0xFF;
	workspace[workspaceIndex++] = 0xD0;

	// If we get here, the Initialize function has been called, 
	// so it's time to close this thread and optionally unload the DLL.

//------------------------------------------//
// Exiting from the injected dll.			//
//------------------------------------------//

// Call ExitThread to leave the DLL loaded
#if 1
#ifdef _WIN64
    //XOR RCX, RCX
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0x31;
    workspace[workspaceIndex++] = 0xC9;

    //MOV RAX, ADDRESS(ExitThread)
    workspace[workspaceIndex++] = 0x48;
    workspace[workspaceIndex++] = 0xB8;
    memcpy(workspace + workspaceIndex, &exitthread, sizeof(exitthread));
    workspaceIndex += sizeof(exitthread);

    //CALL RAX
    workspace[workspaceIndex++] = 0xFF;
    workspace[workspaceIndex++] = 0xD0;
#else
	// Push 0 (exit code)
	workspace[workspaceIndex++] = 0x6A;
	workspace[workspaceIndex++] = 0x00;

	// MOV EAX, ADDRESS - Move the address of ExitThread into EAX
	workspace[workspaceIndex++] = 0xB8;
	memcpy(workspace + workspaceIndex, &exitthread, 4);
	workspaceIndex += 4;

	// CALL EAX - Call ExitThread
	workspace[workspaceIndex++] = 0xFF;
	workspace[workspaceIndex++] = 0xD0;
#endif
#endif

// Call FreeLibraryAndExitThread to unload DLL
#if 0
	// Push 0 (exit code)
	workspace[workspaceIndex++] = 0x6A;
	workspace[workspaceIndex++] = 0x00;

	// PUSH [0x000000] - Push the address of the DLL module to unload
	workspace[workspaceIndex++] = 0xFF;
	workspace[workspaceIndex++] = 0x35;
	memcpy(workspace + workspaceIndex, &dllAddr, 4);
	workspaceIndex += 4;

	// MOV EAX, ADDRESS - Move the address of FreeLibraryAndExitThread into EAX
	workspace[workspaceIndex++] = 0xB8;
	memcpy(workspace + workspaceIndex, &freelibraryandexitthread, 4);
	workspaceIndex += 4;

	// CALL EAX - Call FreeLibraryAndExitThread
	workspace[workspaceIndex++] = 0xFF;
	workspace[workspaceIndex++] = 0xD0;
#endif

//------------------------------------------//
// Code injection and cleanup.				//
//------------------------------------------//

	// Change page protection so we can write executable code
	VirtualProtectEx(hProcess, codecaveAddress, workspaceIndex, PAGE_EXECUTE_READWRITE, &oldProtect);

	// Write out the patch
	WriteProcessMemory(hProcess, codecaveAddress, workspace, workspaceIndex, &bytesRet);

	// Restore page protection
	VirtualProtectEx(hProcess, codecaveAddress, workspaceIndex, oldProtect, &oldProtect);

	// Make sure our changes are written right away
	FlushInstructionCache(hProcess, codecaveAddress, workspaceIndex);

	// Free the workspace memory
	HeapFree(GetProcessHeap(), 0, workspace);

	// Execute the thread now and wait for it to exit, note we execute where the code starts, and not the codecave start
	// (since we wrote strings at the start of the codecave) -- NOTE: void* used for VC6 compatibility instead of UlongToPtr
	return CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((void*)codecaveExecAddr), 0, 0, NULL);
	

	// Free the memory in the process that we allocated
	//VirtualFreeEx(hProcess, codecaveAddress, 0, MEM_RELEASE);
}