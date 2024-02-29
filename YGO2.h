#pragma once

#define YGO2_2006_10 0      // doesn't exist
#define YGO2_2008_01 164
#define YGO2_2008_11 381

#define YGO2_LOGFILE_NAME   "ygo2_dbg.txt"
#define YGO2_LOGFILE_V_NAME "ygo2_dbg_verbose.txt"

#include "YGOShared.hpp"
#include "include/MinHook.h"
#include <locale>
#include <codecvt>

// Address includes
#include "YGO2_ADDR_200610.h"
#include "YGO2_ADDR_200801.h"
#include "YGO2_ADDR_200811.h"

class YGO2 {

public:
    // Generic typedefs for YGO2
    typedef void(__fastcall* hooktype_debuglog)(const char* a, ...); //made to know what are hookable
    typedef void(__fastcall* hooktype_debuglog_net)(const char* a);
    typedef void(__fastcall* hooktype_printf)(const char* a, ...);
    typedef int(*hooktype_fprintf)(FILE* const Stream, const char* const Format, ...);
    typedef int(*hooktype_sprintf)(char* const Buffer, const char* const Format, ...);
    typedef int(__fastcall* hooktype_debuglog_verb)(void*, int, const void*, unsigned int);

    typedef void(__fastcall* hooktype_duelscene)();
    typedef int(__stdcall* hooktype_scn_mainloop)(void*, void*);
    //typedef int(*hooktype_scn_janken)(char a, int i);
    //typedef int(*hooktype_scn_duel)(char a, int i);
    typedef int(__cdecl* hooktype_duelstart)(int);

    // The rest (tm)
    YGO2(int ver, std::string verStr);
    void EmptyStub();
    void __fastcall EmptyStubFast();

    hooktype_duelscene      duelHook;
    hooktype_debuglog       debuglogHook;
    hooktype_debuglog_net   debuglogNetworkHook;
    hooktype_printf         printfHook;
    hooktype_fprintf        fprintfHook;
    hooktype_sprintf        sprintfHook;
    hooktype_debuglog_verb   debuglogVerbHook;

    hooktype_scn_mainloop   sceneMainLoopHook;
    hooktype_duelstart      duelStartHook;

    // detour functions
    static void debug_log(char* msg, ...);
    static void __fastcall network_response_log_stub(const char* a);
    static char* debug_log_buffer_ret(char* msg, ...);
    static int fprint_reimpl(FILE* const Stream, const char* const Format, char* bootleg_va);
    static int sprintf_reimpl(char* const Buffer, const char* const Format, ...);
    static int __fastcall debug_log_verb(void* _this, int a, const char* b, unsigned int c);

    static int __fastcall scene_mainloop_reimpl(void* _this, void* x, int sceneNumber);
    static int __cdecl duel_start_reimpl(int mode);
};