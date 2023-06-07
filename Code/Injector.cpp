#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcId(const wchar_t* procName)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp(procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

std::string GetCurrentPath(int exeLen) // exeLen is the amount of characters in the .exe
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, sizeof(path));
    std::string str = std::string(path);
    str = str.erase(str.length() - exeLen); // remove the .exe from the path
    
    std::string result = str; // add double slahes
    int replacements = 0;
    for (int i = 0; i < str.length(); i++) 
    {
        if(str[i] == '\\')
        {
            result.insert(i+replacements, "\\");
            replacements++;
        }
    }
    return result;
}

int main()
{
    std::string exeName = "PooliansAimBotInjector.exe";
    std::string dir = GetCurrentPath(exeName.length()) + "PooliansAimBot.dll";
    const char* dllPath = dir.c_str();

    DWORD procId = 0;
    while (!procId)
    {
        procId = GetProcId(L"Poolians.exe");
        Sleep(500);
    }

    std::cout << "Found Process: " << procId << "\n";

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

    if (hProc && hProc != INVALID_HANDLE_VALUE)
    {
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);

        std::cout << "DLL Injected at " << loc << "\n";

        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

        if (hThread)
        {
            CloseHandle(hThread);
        }
    }

    if (hProc)
    {
        CloseHandle(hProc);
    }
    return 0;
}