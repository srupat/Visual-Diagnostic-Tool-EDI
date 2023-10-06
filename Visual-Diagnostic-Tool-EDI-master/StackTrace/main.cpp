#include <Windows.h>
#include <TlHelp32.h>
#include <DbgHelp.h>
#include <iostream>
#include <Psapi.h>
#include "StackTrace.h"

#pragma comment(lib, "Dbghelp.lib")

bool LaunchProcess(const wchar_t* targetProcessPath, wchar_t* commandLineArgs = nullptr)
{
    // Security attributes for the process (optional).
    LPSECURITY_ATTRIBUTES lpProcessAttributes = nullptr;

    // Security attributes for the thread (optional).
    LPSECURITY_ATTRIBUTES lpThreadAttributes = nullptr;

    // Set to TRUE to inherit handles from the parent process or FALSE to not inherit.
    BOOL bInheritHandles = FALSE;

    // Flags for the new process (e.g., CREATE_NEW_CONSOLE, CREATE_SUSPENDED, etc.).
    DWORD dwCreationFlags = CREATE_NEW_CONSOLE;

    // Environment variables (optional, nullptr for the current process's environment).
    LPVOID lpEnvironment = nullptr;

    // Current directory for the new process (optional, nullptr for the current process's directory).
    LPWSTR lpCurrentDirectory = nullptr;

    // Startup information for the new process.
    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    // Process information for the new process.
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));

    // Start the new process.
    if (!CreateProcessW(targetProcessPath, commandLineArgs, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
        dwCreationFlags, lpEnvironment, lpCurrentDirectory, &startupInfo, &processInfo))
    {
        std::cerr << "Failed to create the process. Error code: " << GetLastError() << std::endl;
        return false;
    }

    // Close handles to avoid resource leaks.
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return true;
}

int main()
{

    // Replace "C:\\path\\to\\your\\program.exe" with the path to the target program.
    const wchar_t* targetProcessPath = L"C:\\Users\\91878\\source\\repos\\VD_test\\Debug\\VD_test.exe";

    // Additional command-line arguments if needed.
    wchar_t* commandLineArgs = nullptr;

    // Launch the process using the function.
    if (LaunchProcess(targetProcessPath, commandLineArgs))
    {
        std::cout << "Process launched successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to launch the process." << std::endl;
    }




    // Get the list of all running processes.
    DWORD processes[1024];
    DWORD needed;
    if (!EnumProcesses(processes, sizeof(processes), &needed))
    {
        std::cerr << "Failed to enumerate processes." << std::endl;
        return 1;
    }


    // Calculate the number of processes returned.
    DWORD numProcesses = needed / sizeof(DWORD);

    Sleep(900);
    // Iterate through the list of processes.
    for (DWORD i = 0; i < numProcesses; i++)
    {
        // Open each process with the necessary permissions.
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[i]);
        if (hProcess != nullptr)
        {
            TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

            // Get the process name.
            HMODULE hMod;
            DWORD cbNeeded;
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
            {
                if (!GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR)))
                {
                    std::cout << "GetModuleBaseName failed" << std::endl;
                }
            }
            else
            {
              //  std::cout << "EnumProcess module failed" << std::endl;
            }

            // Print the process name and ID.
            //std::wcout << L"Process Name: " << szProcessName << L", Process ID: " << processes[i] << std::endl;


            if (_wcsicmp(szProcessName, L"VD_test.exe") == 0)
            {
                std::cout << "Stack Trace for the process : " << std::endl;
                StackTrace stackTrace;
                stackTrace.printStackTrace(hProcess);
            }

            CloseHandle(hProcess);
        }
    }

    return 0;
}