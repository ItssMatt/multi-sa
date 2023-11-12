/*************************************************************

	This file is the main entry point for the .exe that will
	be used to run the Multiplayer project. It takes the
	'Client' .dll and injects it into the gta_sa.exe file.

	IT MUST BE RUN WITH ADMINISTRATOR PRIVILEDGES!

 *************************************************************/

#include <iostream>
#include <Windows.h>

bool injectDLL(HANDLE hProcess, const char* dllPath) {
    // Allocate memory in the remote process for the DLL path
    LPVOID remoteString = VirtualAllocEx(hProcess, nullptr, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remoteString == nullptr) {
        std::cerr << "VirtualAllocEx failed. Error code: " << GetLastError() << std::endl;
        return false;
    }

    if (!WriteProcessMemory(hProcess, remoteString, dllPath, strlen(dllPath) + 1, nullptr)) {
        std::cerr << "WriteProcessMemory failed. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        return false;
    }

    // Get the address of LoadLibraryA in the kernel32.dll
    LPVOID loadLibraryAddr = reinterpret_cast<LPVOID>(GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));
    if (loadLibraryAddr == nullptr) {
        std::cerr << "GetProcAddress failed. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        return false;
    }

    // Create a remote thread in the target process to load the DLL
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryAddr), remoteString, 0, nullptr);

    if (hThread == nullptr) {
        std::cerr << "CreateRemoteThread failed. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        return false;
    }
    return true;
}

int main() {
    // Getting the files' paths
    const char* executableName = "gta_sa.exe";
    const char* dllName = "Client.dll";

    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);

    std::string executablePath = currentDir;
    executablePath += "\\";
    executablePath += executableName;

    std::string dllPath = currentDir;
    dllPath += "\\";
    dllPath += dllName;

    STARTUPINFOA startupInfo = {};
    PROCESS_INFORMATION processInfo = {};

    if (CreateProcessA(
        nullptr,                  // Application name (use nullptr to use command line)
        const_cast<char*>(executablePath.c_str()), // Command line
        nullptr,                  // Process handle not inheritable
        nullptr,                  // Thread handle not inheritable
        FALSE,                    // Set handle inheritance to FALSE
        0,                        // No creation flags
        nullptr,                  // Use parent's environment block
        nullptr,                  // Use parent's starting directory
        &startupInfo,             // Pointer to STARTUPINFO structure
        &processInfo              // Pointer to PROCESS_INFORMATION structure
    )) {
        std::cout << "Successfully created the process." << std::endl;

        // Inject the DLL into the process
        if (injectDLL(processInfo.hProcess, dllPath.c_str()))
            std::cout << "DLL Injection success. Proceed to the next console..." << std::endl;

        Sleep(3000);
    }
    else {
        std::cerr << "Failed to create the process. Error code: " << GetLastError() << std::endl;
    }
    // Exit after 3 seconds
    return 0;
}