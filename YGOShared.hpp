#pragma once

// Helper class that does not contain funtion reimplementations, but code that can be shared between all versions

#include <iostream>
#include <sstream>
#include <windows.h>
#include <locale.h>
#include <codecvt>

static void log_write(const char* fname, const char* buffer, bool forceNewLine)
{
    char* filename = new char[MAX_PATH], pwd[MAX_PATH];

    GetCurrentDirectoryA(MAX_PATH, pwd);
    sprintf(filename, "%s\\%s", pwd, fname);

    FILE* fd = fopen(filename, "a");
    if (fd == NULL) return;

    fputs(buffer, fd);
    printf(buffer);
    if (forceNewLine) {
        fputs("\n", fd);
        printf("\n");
    }

    fclose(fd);
}

// Hash function for strings
constexpr unsigned int strToHash(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (strToHash(str, h + 1) * 33) ^ str[h];
}