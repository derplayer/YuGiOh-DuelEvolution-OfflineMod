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

static int hexCharToByte(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void hexStringToByteArray(const std::string& hexString, unsigned char* byteArray, size_t byteArraySize) {
    if (hexString.length() % 2 != 0) {
        throw std::invalid_argument("Hex string length must be even");
    }

    for (size_t i = 0; i < byteArraySize; ++i) {
        byteArray[i] = (hexCharToByte(hexString[2 * i]) << 4) | hexCharToByte(hexString[2 * i + 1]);
    }
}


// Hash function for strings
constexpr unsigned int strToHash(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (strToHash(str, h + 1) * 33) ^ str[h];
}