#pragma once

#define YGO_2006_02 "Ver.060110.00"

#define YGO_LOGFILE_NAME   "ygo_dbg.txt"
#define YGO_LOGFILE_V_NAME "ygo_dbg_verbose.txt"

#include "YGOShared.hpp"
#include "include/MinHook.h"
#include <locale>
#include <codecvt>
#include <functional>

// Address includes
#include "YGO_ADDR_200602.h"

class YGO {

public:
    // Generic typedefs for YGO2
    typedef void(__fastcall* hooktype_debuglog)(const char* a, ...); //made to know what are hookable
    typedef int(*hooktype_sprintf)(char* const Buffer, const char* const Format, ...);

    // The rest (tm)
    YGO(std::string verStr);
    void EmptyStub();
    void __fastcall EmptyStubFast();

    hooktype_debuglog       debuglogHook;
    hooktype_sprintf        sprintfHook;

    // detour functions
    static void debug_log(char* msg, ...);
    static int sprintf_reimpl(char* const Buffer, const char* const Format, ...);
};