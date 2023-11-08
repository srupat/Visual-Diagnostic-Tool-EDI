#define _CRT_SECURE_NO_WARNINGS
#include "Export.h"
#include "StackTrace.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <DbgHelp.h>
#include <iostream>
#include <Psapi.h>


static StackTrace* pstackTrace;

extern "C" __declspec(dllexport) BSTR GetFunctionName(int iter) {
    
    std::string s = pstackTrace->strFunctionNames[iter];
    _bstr_t myBstr(s.c_str());
    return myBstr;
}

extern "C" __declspec(dllexport) int GetFunctionCount() {
	    
    if (pstackTrace) {
        delete pstackTrace;
    }
    pstackTrace = new StackTrace();

    // Replace "C:\\path\\to\\your\\program.exe" with the path to the target program.
    const wchar_t* targetProcessPath = L"C:\\Users\\sruja\\source\\repos\\Visual_Test\\Debug\\Visual_Test.exe";

    // Additional command-line arguments if needed.
        wchar_t* commandLineArgs = nullptr;

    // Launch the process using the function.

    /*if (LaunchProcess(targetProcessPath, commandLineArgs))
    {
        std::cout << "Process launched successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to launch the process." << std::endl;
    }*/

    // Get the list of all running processes.
    DWORD processes[1024];
    DWORD needed;
    if (!EnumProcesses(processes, sizeof(processes), &needed))
    {
        std::cerr << "Failed to enumerate processes." << std::endl;
    }

    // Calculate the number of processes returned.
    DWORD numProcesses = needed / sizeof(DWORD);

    //Sleep(900);
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


            if (_wcsicmp(szProcessName, L"VisualTest.exe") == 0)
            {
                std::cout << "Stack Trace for the process : " << std::endl;
                pstackTrace->printStackTrace(hProcess);
            }
            CloseHandle(hProcess);
        }
    }

     int stringCount = (pstackTrace->strFunctionNames).size();
     return stringCount;
}
