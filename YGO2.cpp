﻿#pragma warning(disable:26495)

#include "YGO2.h"
static int ygoVer = 0;

// ### Trampoline returns
static YGO2::hooktype_fprintf fprintfHook_Return = nullptr;
static YGO2::hooktype_sprintf sprintfHook_Return = nullptr;
static YGO2::hooktype_debuglog_verb debuglogVerbHook_Return = nullptr;

static YGO2::hooktype_scn_mainloop sceneMainLoopHook_Return = nullptr;
static YGO2::hooktype_duelstart duelStartHook_Return = nullptr;

// ### Implementations
void YGO2::EmptyStub() {
    return;
}

void __fastcall YGO2::EmptyStubFast()
{
    return;
}

void YGO2::debug_log(char* msg, ...)
{
    char buffer[512];
    sprintf(buffer, "debug log: ");

    va_list args;
    va_start(args, msg);
    vsprintf(buffer + strlen(buffer), msg, args);
    va_end(args);

    printf(buffer);

    if (!strstr(buffer, "\n")) {
        printf("\n");
    }

    log_write(YGO2_LOGFILE_NAME, buffer, false);
    return;
}

void __fastcall YGO2::network_response_log_stub(const char* a)
{
    return;
}

char* YGO2::debug_log_buffer_ret(char* msg, ...)
{
    char buffer[512];

    va_list args;
    va_start(args, msg);
    vsprintf(buffer, msg, args);
    va_end(args);

    printf(buffer);
    if (!strstr(buffer, "\n")) {
        printf("\n");
    }

    log_write(YGO2_LOGFILE_NAME, buffer, true);
    return buffer;
}

int YGO2::fprint_reimpl(FILE* const Stream, const char* const Format, char* bootleg_va)
{
    char bufferA[512];
    sprintf(bufferA, "fprintf HOOK: ");
    sprintf(bufferA + strlen(bufferA), Format, bootleg_va);
    log_write(YGO2_LOGFILE_NAME, bufferA, true);

    return fprintfHook_Return(Stream, Format, bootleg_va);
}

int YGO2::sprintf_reimpl(char* const Buffer, const char* const Format, char* bootleg_va)
{
    //printf(Format);
    char bufferA[512];
    sprintf(bufferA, "sprintf HOOK: ");
    sprintf(bufferA + strlen(bufferA), Format, bootleg_va);
    log_write("ygo2_dbg.txt", bufferA, true);

    return sprintfHook_Return(Buffer, Format, bootleg_va);
}

// INFO: bootleg thiscall to fastcall, x is "thiscall" ecx register
int __fastcall  YGO2::debug_log_verb(void* _this, int a, const char* b, unsigned int c)
{
    // do not print empty msgs
    if (strlen(b) <= 1) return debuglogVerbHook_Return(_this, a, b, c);

    // Spam decrease
    if (strstr(b, "Shape") != NULL)         return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "Rotate") != NULL)        return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "Scale") != NULL)         return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "Flags") != NULL)         return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "Depth") != NULL)         return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "CTextElm") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "Xmin") != NULL)          return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "Ymin") != NULL)          return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "FileName") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "CharacterID") != NULL)   return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "Sprite") != NULL)        return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "MultTerm") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "unknown") != NULL)       return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "GameLoop") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "GameStep") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "_btn") != NULL)          return debuglogVerbHook_Return(_this, a, b, c);
    if (strstr(b, "ﾌﾌﾌﾌﾌﾌﾌﾌ") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);

    char bufferA[512];
    sprintf(bufferA, "verbose HOOK: ");
    sprintf(bufferA + strlen(bufferA), b, a);

    log_write(YGO2_LOGFILE_V_NAME, bufferA, true);

    return debuglogVerbHook_Return(_this, a, b, c);
}

int(__thiscall* scene_mainloop)(void*, int);
int __fastcall YGO2::scene_mainloop_reimpl(void* _this, void* x, int sceneNumber) {
    // here we are calling the trampoline & executing the code
    // of the target function, while also being able to run our own code afterwards/before

    char scnStr[32];
    sprintf(scnStr, "Loading scene id: %d", sceneNumber);
    log_write("ygo2_dbg.txt", scnStr, true);

    // override scene
    // important scene ids: 11,12 (deck editor)
    // 15 (debug menu)

    //int* duelModeDword = (int*)0x012A9084; // 200811
    //sceneNumber = 15; // forces debug menu at start

    // Disable this hook after one usage
    switch (ygoVer)
    {
    case YGO2_2006_10:
        //MH_DisableHook(hooktype_scn_mainloop(SCN_MAINLOOP_200610));
        break;
    case YGO2_2008_01:
        //MH_DisableHook(hooktype_scn_mainloop(SCN_MAINLOOP_200801));
        break;
    case YGO2_2008_11:
        //MH_DisableHook(hooktype_scn_mainloop(SCN_MAINLOOP_200811));
        break;
    }

    return scene_mainloop(_this, sceneNumber);
}

int(__cdecl* duel_start)(int);
int __cdecl YGO2::duel_start_reimpl(int mode) {
    int* duelModeDword = (int*)0x012A9084;
    int* duelTimerDword = (int*)0x012A9080;

    // modes: 0 (single duel), 1 (match duel) 2,5,9,12
    // mode = 4;
    return duel_start(mode);
}

// ### CONSTRUCTOR
YGO2::YGO2(int ver) {
    ygoVer = ver;

    // Debug console
    AllocConsole();
    SetConsoleOutputCP(932);
    SetConsoleCP(932);
    std::wcout.imbue(std::locale("ja_jp.utf-8"));
    SetConsoleTitleA("YGO2 DEBUG CONSOLE");
    freopen("CONOUT$", "w", stdout);

    std::cout << "YGO2 (ver " << ygoVer << ") detected!" << std::endl;

    // Debug menu change text
    wchar_t* debugTextOverridePtr;
    char* debugParam;
    std::wstringstream debugWStrStream;
    debugWStrStream << "@9 Yu-Gi-Oh! Online: Duel Evolution (build ver " << ygoVer << ")\
                    \n@9 Hotkeys for additional debug output: SHIFT+W, SHIFT+E, SHIFT+X, SHIFT+C \
                    \n@9 Press the key '1' to load a deck from memory. Clicking the duel button will only create a dummy deck. \
                    \n@1 You need a kaban.bin to enable cards in deck builder! \
                    \n\n@7 Created by DerPlayer and PhilYeahz";

    switch (ver)
    {
    case YGO2_2006_10:
        break;
    case YGO2_2008_01:
        debuglogHook = hooktype_debuglog(DEBUG_LOG_ADDR_200801);
        debuglogNetworkHook = hooktype_debuglog_net(DEBUG_LOG_ADDR_NETWORK_200801);
        printfHook = hooktype_printf(DEBUG_LOG_PRINTF_200801);
        fprintfHook = hooktype_fprintf(DEBUG_LOG_FPRINTF_200801);
        sprintfHook = hooktype_sprintf(DEBUG_LOG_SPRINTF_200801);
        debuglogVerbHook = hooktype_debuglog_verb(DEBUG_LOG_VERB_200801);

        //sceneMainLoopHook = hooktype_scn_mainloop(SCN_MAINLOOP_200801);
        break;
    case YGO2_2008_11:
        // Force activate debug mode by nulling the param string
        debugParam = (char*)DEBUG_PARAMFLAG_200811;
        strncpy(debugParam, "", 5);

        // Text edit 200811
        debugTextOverridePtr = (wchar_t*)DEBUG_TEXTSTRING_200811;
        wcsncpy(debugTextOverridePtr, debugWStrStream.str().c_str(), 420); // ~426 is max!

        debuglogHook = hooktype_debuglog(DEBUG_LOG_ADDR_200811);
        debuglogNetworkHook = hooktype_debuglog_net(DEBUG_LOG_ADDR_NETWORK_200811);
        printfHook = hooktype_printf(DEBUG_LOG_PRINTF_200811);
        fprintfHook = hooktype_fprintf(DEBUG_LOG_FPRINTF_200811);
        sprintfHook = hooktype_sprintf(DEBUG_LOG_SPRINTF_200811);
        debuglogVerbHook = hooktype_debuglog_verb(DEBUG_LOG_VERB_200811);

        sceneMainLoopHook = hooktype_scn_mainloop(SCN_MAINLOOP_200811);
        //duelHook = hooktype_duelscene(DUEL_ADDR_200811);
        //duelStartHook = hooktype_duelstart(DUEL_START_200811); - corrupts the game too much
        break;
    default:
        MessageBoxW(0, L"This YGO2 game version is not yet supported by the DLL plugin. Please submit it.", L"", 0);
        return;
    }
    MH_STATUS dlogRes;

    // BASE #00 - Stub log restoration Hook
    dlogRes = MH_CreateHook(debuglogHook, &debug_log, NULL);
    if (dlogRes == MH_OK) MH_EnableHook(debuglogHook);

    // BASE #01 - Network log
    dlogRes = MH_CreateHook(debuglogNetworkHook, &network_response_log_stub, NULL);
    if (dlogRes == MH_OK) MH_EnableHook(debuglogNetworkHook);

    // BASE #02 - printf
    dlogRes = MH_CreateHook(printfHook, &debug_log_buffer_ret, NULL);
    if (dlogRes == MH_OK) MH_EnableHook(printfHook);

    // BASE #03 - fprintf - needs tramp. return
    dlogRes = MH_CreateHook(fprintfHook, &fprint_reimpl, reinterpret_cast<LPVOID*>(&fprintfHook_Return));
    if (dlogRes == MH_OK) MH_EnableHook(fprintfHook);

    // BASE #04 - sprintf - needs tramp. return
    dlogRes = MH_CreateHook(sprintfHook, &sprintf_reimpl, reinterpret_cast<LPVOID*>(&sprintfHook_Return));
    if (dlogRes == MH_OK) MH_EnableHook(sprintfHook);

    // BASE #05 - Stub log restoration (error) Hook - (disabled by default because slows the app too much)
    //dlogRes = MH_CreateHook(debuglogVerbHook, &debug_log_verb, reinterpret_cast<LPVOID*>(&debuglogVerbHook_Return));
    //if (dlogRes == MH_OK) MH_EnableHook(debuglogVerbHook);

    // Extra #01 - Scene main loop (with scene id)
    dlogRes = MH_CreateHook(
        sceneMainLoopHook, // target
        reinterpret_cast<void*>(&scene_mainloop_reimpl),
        reinterpret_cast<void**>(&scene_mainloop)
    );

    if (dlogRes == MH_OK) MH_EnableHook(sceneMainLoopHook);

    // Extra #02 - Duel start (with duel mode id)
    dlogRes = MH_CreateHook(duelStartHook, &duel_start_reimpl, reinterpret_cast<LPVOID*>(&duelStartHook_Return));
    if (dlogRes == MH_OK) MH_EnableHook(duelStartHook);
}