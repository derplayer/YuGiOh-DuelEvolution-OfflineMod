#pragma once

#define YGO2_2006_10 0      // doesn't exist
#define YGO2_2007_03 32     // doesn't exist
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
#include "YGO2_ADDR_200703.h"
#include "YGO2_ADDR_200801.h"
#include "YGO2_ADDR_200811.h"

class YGO2 {

public:
    // Generic typedefs for YGO2
    typedef char(__fastcall* hooktype_sub_5ADCB0)(unsigned int a, unsigned int b);
    typedef int(__cdecl* hooktype_sub_64F8E0)(int a, int b, int c, unsigned int d, unsigned int e);
    typedef void(__fastcall* hooktype_debuglog)(const char* a, ...); //made to know what are hookable
    typedef void(__fastcall* hooktype_debuglog_net)(const char* a);
    typedef void(__fastcall* hooktype_printf)(const char* a, ...);
    typedef int(*hooktype_fprintf)(FILE* const Stream, const char* const Format, ...);
    typedef int(*hooktype_sprintf)(char* const Buffer, const char* const Format, ...);
    typedef int(__fastcall* hooktype_debuglog_verb)(void*, int, const void*, unsigned int);
    typedef void* (__cdecl* hooktype_debuglog_file)(FILE* Stream, int, char);

    typedef void(__fastcall* hooktype_duelscene)();
    typedef int(__stdcall* hooktype_scn_mainloop)(void*, void*);
    //typedef int(*hooktype_scn_janken)(char a, int i);
    //typedef int(*hooktype_scn_duel)(char a, int i);
    typedef int(__cdecl* hooktype_duelstart)(int);
    typedef int(__cdecl* hooktype_dueldeck)(int);
    //typedef DWORD* (__cdecl* hooktype_dueldeck)(int);

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
    hooktype_debuglog_verb  debuglogVerbHook;
    hooktype_debuglog_file  debuglogFileHook;

    hooktype_scn_mainloop   sceneMainLoopHook;
    hooktype_duelstart      duelStartHook;
    hooktype_dueldeck       duelDeckHook;

    hooktype_sub_5ADCB0     sub_5adcb0_hook;
    hooktype_sub_64F8E0     sub_64f8e0_hook;

    // detour functions
    static void debug_log(char* msg, ...);
    static void __fastcall network_response_log_stub(const char* a);
    static char* debug_log_buffer_ret(char* msg, ...);
    static int fprint_reimpl(FILE* const Stream, const char* const Format, char* bootleg_va);
    static int sprintf_reimpl(char* const Buffer, const char* const Format, ...);
    static int __fastcall debug_log_verb(void* _this, int a, const char* b, unsigned int c);

    static int __fastcall scene_mainloop_reimpl(void* _this, void* x, int sceneNumber);
    static int __cdecl duel_start_reimpl(int mode);
    static int __cdecl duel_deck_prepare_reimpl(int player, int, int);

    static char __fastcall sub_5adcb0_reimpl(unsigned int a, unsigned int b);
    static int __cdecl sub_64f8e0_reimpl(int a, int b, int c, unsigned int d, unsigned int e);
};