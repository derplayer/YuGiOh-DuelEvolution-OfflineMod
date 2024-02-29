#pragma warning(disable:26495)

#include "YGO.h"
static std::string ygoVerStr = "unk";

// ### Trampoline returns
static YGO::hooktype_sprintf sprintfHook_Return = nullptr;

// ### Implementations
void YGO::EmptyStub() {
    return;
}

void __fastcall YGO::EmptyStubFast()
{
    return;
}

void YGO::debug_log(char* msg, ...)
{
    // INFO: at duel, sometimes there is garbage in log call? (atleast in YGO2_2006_10, maybe due to diff. call conv "__cdecl")
    if (msg == NULL || msg == (char*)0x1) {
        printf("Invalid message in stub memory register leftover detected.\n");
        return;
    }

    char buffer[512];
    sprintf(buffer, "debug log: ");

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), msg, args);
    va_end(args);

    // Check if buffer ends with a newline (logs have always newline) - filters out garbage logs
    if (buffer[strlen(buffer) - 1] == '\n') {
        printf(buffer);

        if (!strstr(buffer, "\n")) {
            printf("\n");
        }

        log_write(YGO_LOGFILE_NAME, buffer, false);
        return;
    }

    return;
}

int YGO::sprintf_reimpl(char* const Buffer, const char* const Format, ...)
{
    va_list args;
    va_start(args, Format);

    char bufferA[256];
    snprintf(bufferA, sizeof(bufferA), "sprintf HOOK: %s", Format);

    // Create a separate buffer for the formatted string
    char formattedString[256];
    vsnprintf(formattedString, sizeof(formattedString), Format, args);
    strncat(bufferA, formattedString, sizeof(bufferA) - strlen(bufferA) - 1);
    log_write(YGO_LOGFILE_NAME, formattedString, true);

    // Pass the pre-formatted string to sprintfHook_Return to parse it again dawg (HOLY HAX)
    int result = sprintfHook_Return(Buffer, "%s", formattedString);

    va_end(args);

    return result;
}

// ### CONSTRUCTOR
YGO::YGO(std::string verStr) {
    ygoVerStr = verStr;

    // Debug console
    AllocConsole();
    SetConsoleOutputCP(932);
    SetConsoleCP(932);
    std::wcout.imbue(std::locale("ja_jp.utf-8"));
    SetConsoleTitleA("YGO DEBUG CONSOLE");
    freopen("CONOUT$", "w", stdout);

    std::cout << "YGO (" << ygoVerStr << ") detected!" << std::endl;

    switch (strToHash(ygoVerStr.c_str()))
    {
        case strToHash(YGO_2006_02):
            debuglogHook = hooktype_debuglog(DEBUG_LOG_ADDR_200602);
            sprintfHook = hooktype_sprintf(DEBUG_LOG_SPRINTF_200602);
            break;
        case -1:
            MessageBoxW(0, L"This YGO game version is not yet supported by the DLL plugin. Please submit it.", L"", 0);
            return;
    }
    MH_STATUS dlogRes;

    // BASE #00 - Stub log restoration Hook
    dlogRes = MH_CreateHook(debuglogHook, &debug_log, NULL);
    if (dlogRes == MH_OK) MH_EnableHook(debuglogHook);

    // BASE #01 - sprintf - needs tramp. return
    dlogRes = MH_CreateHook(sprintfHook, &sprintf_reimpl, reinterpret_cast<LPVOID*>(&sprintfHook_Return));
    if (dlogRes == MH_OK) MH_EnableHook(sprintfHook);
}