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

static void PrintMemory(DWORD_PTR baseAddress, char* printLabel)
{
    HANDLE hProcess = GetCurrentProcess();
    BYTE buffer[256];
    // Read the next 256 bytes from the base address
    if (!ReadProcessMemory(hProcess, (LPCVOID)baseAddress, buffer, sizeof(buffer), NULL)) {
        MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
    }

    // Output the data for verification purposes
    printf(printLabel);
    for (int i = 0; i < 256; ++i) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
}

static void PrintMemoryVariable(DWORD_PTR baseAddress, SIZE_T byteLength, char* printLabel)
{
    HANDLE hProcess = GetCurrentProcess();
    BYTE* buffer = (BYTE*)malloc(byteLength);

    if (!ReadProcessMemory(hProcess, (LPCVOID)baseAddress, buffer, byteLength, NULL)) {
        MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
    }

    // Output the data for verification purposes
    printf(printLabel);
    for (SIZE_T i = 0; i < byteLength; ++i) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    free(buffer);
}

static void ApplyTextToNewOffset(const char* newString, const DWORD* offsets, size_t offsetCount) {
    // Allocate memory for the new string
    char* allocatedMemory = (char*)VirtualAlloc(NULL, strlen(newString) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (allocatedMemory != NULL) {
        // Copy the new string to the allocated memory
        strcpy(allocatedMemory, newString);

        // Iterate through the array of offsets and update each one
        for (size_t i = 0; i < offsetCount; ++i) {
            DWORD oldProtect;
            VirtualProtect((LPVOID)offsets[i], sizeof(char*), PAGE_EXECUTE_READWRITE, &oldProtect);

            // Set the new memory address to the instruction
            *(char**)offsets[i] = allocatedMemory;

            VirtualProtect((LPVOID)offsets[i], sizeof(char*), oldProtect, &oldProtect);
        }
    }
    else {
        // Handle allocation error
    }
}

static void ApplyBytesDirect(DWORD dest, const BYTE* src, size_t n) {
    memcpy((void*)dest, src, n);
}

static void ApplyBytesToNewOffset(const BYTE* newBytes, size_t byteCount, const DWORD* offsets, size_t offsetCount) {
    // Allocate memory for the new byte array
    BYTE* allocatedMemory = (BYTE*)VirtualAlloc(NULL, byteCount, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (allocatedMemory != NULL) {
        // Copy the new byte array to the allocated memory
        memcpy(allocatedMemory, newBytes, byteCount);

        // Iterate through the array of offsets and update each one
        for (size_t i = 0; i < offsetCount; ++i) {
            DWORD oldProtect;
            VirtualProtect((LPVOID)offsets[i], sizeof(BYTE*), PAGE_EXECUTE_READWRITE, &oldProtect);

            // Set the new memory address to the instruction
            *(BYTE**)offsets[i] = allocatedMemory;

            VirtualProtect((LPVOID)offsets[i], sizeof(BYTE*), oldProtect, &oldProtect);
        }
    }
    else {
        // Handle allocation error
    }
}

static POINT GetMousePositionInWindow() {
    HWND hWnd = GetForegroundWindow(); // Get the handle of the active window
    POINT point;

    // Get the cursor position in screen coordinates
    if (GetCursorPos(&point)) {
        // Convert the screen coordinates to client coordinates
        if (ScreenToClient(hWnd, &point)) {
            return point;
        }
    }

    // Return an invalid point if the functions fail
    return { -1, -1 };
}

static void LoadDeckFromFileToMemory(HANDLE hProcess, LPCVOID baseAddress, const char* filePath) {
    BYTE buffer[250]; // Buffer to store the 250 bytes

    // Open the file and read the 250 bytes
    FILE* file = fopen(filePath, "rb");
    if (file != NULL) {
        fread(buffer, sizeof(BYTE), sizeof(buffer), file);
        fclose(file);

        // Write the buffer to the specified memory address
        if (!WriteProcessMemory(hProcess, (LPVOID)baseAddress, buffer, sizeof(buffer), NULL)) {
            MessageBox(NULL, "Failed to load the player deck to memory", "Error", MB_OK | MB_ICONERROR);
        }
        else {
            // Print the buffer
            printf("Loaded from %s\n", filePath);
            for (int i = 0; i < sizeof(buffer); ++i) {
                printf("%02X ", buffer[i]);
                if ((i + 1) % 16 == 0) {
                    printf("\n");
                }
            }
        }
    }
    else {
        MessageBox(NULL, "Failed to open player deck file for reading", "Error", MB_OK | MB_ICONERROR);
    }
}

static void WriteDeckFromMemoryToFile(HANDLE hProcess, LPCVOID baseAddress, const char* filePath) {
    BYTE buffer[250]; // Buffer to store the 250 bytes

    // Read 250 bytes from memory
    if (!ReadProcessMemory(hProcess, baseAddress, buffer, sizeof(buffer), NULL)) {
        MessageBox(NULL, "Failed to read player deck from memory", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Write the buffer to the specified file
    FILE* file = fopen(filePath, "wb");
    if (file != NULL) {
        fwrite(buffer, sizeof(BYTE), sizeof(buffer), file);
        fclose(file);

        // Print the buffer
        printf("Player deck written from memory to %s\n", filePath);
        for (int i = 0; i < sizeof(buffer); ++i) {
            printf("%02X ", buffer[i]);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
    } else {
        MessageBox(NULL, "Failed to open player deck file for writing", "Error", MB_OK | MB_ICONERROR);
    }
}

// Hash function for strings
constexpr unsigned int strToHash(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (strToHash(str, h + 1) * 33) ^ str[h];
}