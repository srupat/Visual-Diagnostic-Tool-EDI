#include <Windows.h>
#include <TlHelp32.h>
#include <DbgHelp.h>
#include <iostream>
#include <Psapi.h>
#include "StackTrace.h"

#pragma comment(lib, "Dbghelp.lib")

StackTrace::StackTrace()
{

}

StackTrace::~StackTrace()
{
    for (UINT i = 0; i < m_frames.size(); ++i)
        delete m_frames[i];
}

void StackTrace::printStackTrace(HANDLE hProcess)
{
    // Create a SYMBOL_INFO structure for the stack trace.
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    // Initialize the symbol handler.
    bool sym = SymInitialize(hProcess, "C:\\Users\\91878\\source\\repos\\VD_test\\Debug" , TRUE);

    // Get the thread list.
    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    Thread32First(hThreadSnap, &te);

    do
    {
        if (te.th32OwnerProcessID == GetProcessId(hProcess))
        {
            // Open the thread to get its context.
            HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
            if (hThread)
            {
                CONTEXT context;
                context.ContextFlags = CONTEXT_FULL;
                SuspendThread(hThread);
                if (GetThreadContext(hThread, &context))
                {
                    // Read the stack.
                    DWORD machineType;
                    STACKFRAME64 stackFrame;
                    ZeroMemory(&stackFrame, sizeof(STACKFRAME64));

#if defined(_M_IX86)
                    machineType = IMAGE_FILE_MACHINE_I386;
                    stackFrame.AddrPC.Offset = context.Eip;
                    stackFrame.AddrFrame.Offset = context.Ebp;
                    stackFrame.AddrStack.Offset = context.Esp;
#elif defined(_M_X64)
                    machineType = IMAGE_FILE_MACHINE_AMD64;
                    stackFrame.AddrPC.Offset = context.Rip;
                    stackFrame.AddrFrame.Offset = context.Rsp;
                    stackFrame.AddrStack.Offset = context.Rsp;
#endif
                    stackFrame.AddrPC.Mode = AddrModeFlat;
                    stackFrame.AddrFrame.Mode = AddrModeFlat;
                    stackFrame.AddrStack.Mode = AddrModeFlat;

                    //  m_frames.push_back(new StackFrame(stackFrame, hProcess));
                    struct Node* head = NULL;
                    // Print the stack trace.
                    while (StackWalk64(machineType, hProcess, hThread, &stackFrame, &context, nullptr,
                        SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
                    {

                        StackFrame* frame = new StackFrame(stackFrame, hProcess);

                        if (frame->getContextFlag() == 1)
                        {
                            frame->ToString();
                            printf("\n");
                            m_frames.push_back(frame);
                            SymFromAddr(hProcess, stackFrame.AddrPC.Offset, nullptr, symbol);
                            head = frame->createDataStructure(head);
                        }
                    }
                }
                ResumeThread(hThread);
                CloseHandle(hThread);
            }
        }
    } while (Thread32Next(hThreadSnap, &te));

    SymCleanup(hProcess);
    free(symbol);
    CloseHandle(hThreadSnap);

}