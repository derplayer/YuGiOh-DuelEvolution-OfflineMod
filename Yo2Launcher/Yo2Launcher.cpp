// Yo2Launcher.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>

// Define function pointers for TitanEngine functions
typedef void* (*InitDebug_t)(char*, char*, char*);
typedef BOOL(*AttachDebugger_t)(DWORD);
typedef BOOL(*DebugLoop_t)(void*);
typedef void (*SetCustomHandler_t)(void*);

void CheckYear() {
    // Get current time
    time_t currentTime = time(NULL);
    tm localTime;
    localtime_s(&localTime, &currentTime);

    // Check the year
    int year = localTime.tm_year + 1900; // tm_year is years since 1900
    if (year >= 2025) {
        MessageBox(NULL, L"The Duel Evolution Offline mod alpha test has expired. Please update to newest version.", L"Error", MB_OK | MB_ICONERROR);
        ExitProcess(1); // Exit the application
    }
}

void AttachTitanDebugger() {
    HMODULE hTitanEngine = LoadLibrary(L"TitanEngine.dll");
    if (!hTitanEngine) {
        MessageBox(NULL, L"Failed to load TitanEngine.dll", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Get the function addresses
    InitDebug_t InitDebug = (InitDebug_t)GetProcAddress(hTitanEngine, "InitDebug");
    AttachDebugger_t AttachDebugger = (AttachDebugger_t)GetProcAddress(hTitanEngine, "AttachDebugger");
    DebugLoop_t DebugLoop = (DebugLoop_t)GetProcAddress(hTitanEngine, "DebugLoop");
    SetCustomHandler_t SetCustomHandler = (SetCustomHandler_t)GetProcAddress(hTitanEngine, "SetCustomHandler");

    if (!InitDebug || !AttachDebugger || !DebugLoop || !SetCustomHandler) {
        MessageBox(NULL, L"Failed to get function addresses from TitanEngine.dll", L"Error", MB_OK | MB_ICONERROR);
        FreeLibrary(hTitanEngine);
        return;
    }

    // Get the full path to the executable dynamically
    char szFileName[MAX_PATH];
    if (!GetModuleFileNameA(NULL, szFileName, MAX_PATH)) {
        MessageBoxA(NULL, "Failed to get the module file name", "Error", MB_OK | MB_ICONERROR);
        FreeLibrary(hTitanEngine);
        return;
    }

    // Set the current folder
    char szCurrentFolder[MAX_PATH];
    strcpy_s(szCurrentFolder, MAX_PATH, szFileName);
    *strrchr(szCurrentFolder, '\\') = '\0';  // Remove the executable name to get the folder path

    // Manually set the target executable name
    char targetExe[MAX_PATH];
    sprintf_s(targetExe, MAX_PATH, "%s\\yo2.exe", szCurrentFolder);

    char szCommandLine[] = "";  // Command line parameters, if any
    PROCESS_INFORMATION* pi = (PROCESS_INFORMATION*)InitDebug(targetExe, szCommandLine, szCurrentFolder);
    if (!pi) {
        MessageBoxA(NULL, "Failed to initialize debugging with TitanEngine", "Error", MB_OK | MB_ICONERROR);
        FreeLibrary(hTitanEngine);
        return;
    }


    DWORD processId = pi->dwProcessId;

    //// Attach debugger to the process
    //if (!AttachDebugger(processId)) {
    //    MessageBox(NULL, L"Failed to attach debugger with TitanEngine", L"Error", MB_OK | MB_ICONERROR);
    //    FreeLibrary(hTitanEngine);
    //    return;
    //}

    // Optionally set a custom exception handler
    //SetCustomHandler(NULL);

    // Start the debug loop
    if (!DebugLoop(NULL)) {
        MessageBox(NULL, L"Failed to start debug loop with TitanEngine", L"Error", MB_OK | MB_ICONERROR);
    }

    FreeLibrary(hTitanEngine);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    CheckYear();
    AttachTitanDebugger();
    std::cout << "Yu-Gi-Oh 2 debugger enforced! You shouldn't see this message! If you do, then well... it is what it is.\n";
    return 0;
}
