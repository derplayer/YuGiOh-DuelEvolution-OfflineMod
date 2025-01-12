#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <tlhelp32.h>
#include <iostream>
#include <DbgHelp.h>

namespace fs = std::filesystem;

#include "Detours/src/detours.h"
#ifdef _DEBUG
//#pragma comment(lib, "include/Debug/MinHook.x86.lib")
//#pragma comment(lib, "include/libMinHook-x86-v141-mtd.lib")
//#pragma comment(lib, "lib/Debug/libMinHook.x86.lib")
#else
//#pragma comment(lib, "include/Release/MinHook.x86.lib")
//#pragma comment(lib, "include/libMinHook-x86-v141-mt.lib")
//#pragma comment(lib, "lib/Release/libMinHook.x86.lib")
#endif

//#pragma comment(lib, "include/MinHook.x86.dll")
#include "include/MinHook.h"
#include "Util.hpp"
#include "dllstub.hpp"
#include "DefConfigFile.hpp"

#include "YGO2.h"
#include "YGO.h"

#define CONFIG_FILE L"FontMod.yaml"
#define LOG_FILE L"FontMod.log"

auto addrCreateFontIndirectExW = CreateFontIndirectExW;
auto addrGetStockObject = GetStockObject;

// overrideflags
#define	_NONE 0
#define	_HEIGHT 1 << 1
#define	_WIDTH 1 << 2
#define	_WEIGHT 1 << 3
#define	_ITALIC 1 << 4
#define	_UNDERLINE 1 << 5
#define	_STRIKEOUT 1 << 6
#define	_CHARSET 1 << 7
#define	_OUTPRECISION 1 << 8
#define	_CLIPPRECISION 1 << 9
#define	_QUALITY 1 << 10
#define	_PITCHANDFAMILY 1 << 11

struct font
{
	std::wstring replace;
	uint32_t overrideFlags;
	long height;
	long width;
	long weight;
	bool italic;
	bool underLine;
	bool strikeOut;
	BYTE charSet;
	BYTE outPrecision;
	BYTE clipPrecision;
	BYTE quality;
	BYTE pitchAndFamily;
};

enum GSOFontMode {
	DISABLED,
	USE_NCM_FONT, // Use default font from SystemParametersInfo SPI_GETNONCLIENTMETRICS
	USE_USER_FONT // Use user defined font
};

std::unordered_map<std::wstring, font> fontsMap;
FILE *logFile = nullptr;
HFONT newGSOFont = nullptr;

HFONT WINAPI MyCreateFontIndirectExW(const ENUMLOGFONTEXDVW* lpelf)
{
	auto lplf = &lpelf->elfEnumLogfontEx.elfLogFont;

	if (logFile)
	{
		std::string name;
		if (Utf16ToUtf8(lplf->lfFaceName, name))
		{
#define bool_string(b) b != FALSE ? "true" : "false"
			fprintf_s(logFile,
				"[CreateFont] name = \"%s\", height = %d, "
				"width = %d, escapement = %d, "
				"orientation = %d, weight = %d, "
				"italic = %s, underline = %s, "
				"strikeout = %s, charset = %d, "
				"outprecision = %d, clipprecision = %d, "
				"quality = %d, pitchandfamily = %d\n",
				name.c_str(), lplf->lfHeight,
				lplf->lfWidth, lplf->lfEscapement,
				lplf->lfOrientation, lplf->lfWeight,
				bool_string(lplf->lfItalic), bool_string(lplf->lfUnderline),
				bool_string(lplf->lfStrikeOut), lplf->lfCharSet,
				lplf->lfOutPrecision, lplf->lfClipPrecision,
				lplf->lfQuality, lplf->lfPitchAndFamily);
		}
	}

	ENUMLOGFONTEXDVW elf;

	auto it = fontsMap.find(lplf->lfFaceName);
	if (it != fontsMap.end())
	{
		elf = *lpelf;
		LOGFONTW& lf = elf.elfEnumLogfontEx.elfLogFont;

		size_t len = it->second.replace._Copy_s(lf.lfFaceName, LF_FACESIZE, LF_FACESIZE);
		lf.lfFaceName[len] = L'\0';

		if ((it->second.overrideFlags & _HEIGHT) == _HEIGHT)
			lf.lfHeight = it->second.height;
		if ((it->second.overrideFlags & _WIDTH) == _WIDTH)
			lf.lfWidth = it->second.width;
		if ((it->second.overrideFlags & _WEIGHT) == _WEIGHT)
			lf.lfWeight = it->second.weight;
		if ((it->second.overrideFlags & _ITALIC) == _ITALIC)
			lf.lfItalic = it->second.italic;
		if ((it->second.overrideFlags & _UNDERLINE) == _UNDERLINE)
			lf.lfUnderline = it->second.underLine;
		if ((it->second.overrideFlags & _STRIKEOUT) == _STRIKEOUT)
			lf.lfStrikeOut = it->second.strikeOut;
		if ((it->second.overrideFlags & _CHARSET) == _CHARSET)
			lf.lfCharSet = it->second.charSet;
		if ((it->second.overrideFlags & _OUTPRECISION) == _OUTPRECISION)
			lf.lfOutPrecision = it->second.outPrecision;
		if ((it->second.overrideFlags & _CLIPPRECISION) == _CLIPPRECISION)
			lf.lfClipPrecision = it->second.clipPrecision;
		if ((it->second.overrideFlags & _QUALITY) == _QUALITY)
			lf.lfQuality = it->second.quality;
		if ((it->second.overrideFlags & _PITCHANDFAMILY) == _PITCHANDFAMILY)
			lf.lfPitchAndFamily = it->second.pitchAndFamily;

		lpelf = &elf;
	}
	return addrCreateFontIndirectExW(lpelf);
}

HGDIOBJ WINAPI MyGetStockObject(int i)
{
	switch (i)
	{
	case OEM_FIXED_FONT:
	case ANSI_FIXED_FONT:
	case ANSI_VAR_FONT:
	case SYSTEM_FONT:
	case DEVICE_DEFAULT_FONT:
	case SYSTEM_FIXED_FONT:
		return newGSOFont;
	}
	return addrGetStockObject(i);
}

void SetupYGOFontRendering() {
	font fontInfo;
	std::wstring originalFont = L"Arial";
	std::wstring replacementFont = L"Yu Gothic UI Bold"; //Yu Gothic UI, MS UI Gothic, Microsoft JhengHei UI, Microsoft YaHei UI

	fontInfo.replace = replacementFont;

	// Change the flags how you want

	fontInfo.overrideFlags |= _WEIGHT;
	fontInfo.overrideFlags |= _ITALIC;
	fontInfo.overrideFlags |= _UNDERLINE;
	fontInfo.overrideFlags |= _STRIKEOUT;
	fontInfo.overrideFlags |= _CHARSET;
	fontInfo.overrideFlags |= _OUTPRECISION;
	fontInfo.overrideFlags |= _CLIPPRECISION;
	fontInfo.overrideFlags |= _QUALITY;
	fontInfo.overrideFlags |= _PITCHANDFAMILY;

	fontInfo.overrideFlags |= _WIDTH;
	fontInfo.width = 8;
	fontInfo.overrideFlags |= _HEIGHT;
	fontInfo.height = 18;

	// FW_DONTCARE	0
	// FW_THIN	100
	// FW_EXTRALIGHT	200
	// FW_LIGHT	300
	// FW_NORMAL	400
	// FW_MEDIUM	500
	// FW_SEMIBOLD	600
	// FW_BOLD	700
	// FW_ULTRABOLD	800
	// FW_HEAVY	900
	fontInfo.weight = FW_BOLD;
	fontInfo.italic = false;
	fontInfo.underLine = false;
	fontInfo.strikeOut = false;
	fontInfo.charSet = SHIFTJIS_CHARSET;
	fontInfo.outPrecision = OUT_TT_ONLY_PRECIS;
	fontInfo.clipPrecision = CLIP_DEFAULT_PRECIS;
	fontInfo.quality = CLEARTYPE_QUALITY;
	fontInfo.pitchAndFamily = FIXED_PITCH | FF_MODERN; // VARIABLE_PITCH FIXED_PITCH

	// Add the font info to the map
	fontsMap[originalFont] = fontInfo;
}

void LoadUserFonts(const fs::path& path)
{
	try
	{
		auto fontsPath = path / L"fonts";
		if (fs::is_directory(fontsPath))
		{
			for (auto& f : fs::directory_iterator(fontsPath))
			{
				if (f.is_directory()) continue;
				int ret = AddFontResourceExW(f.path().c_str(), FR_PRIVATE, 0);
				if (logFile)
				{
					fprintf_s(logFile, "[LoadUserFonts] filename = \"%s\", ret = %d, lasterror = %d\n", f.path().filename().u8string().c_str(), ret, GetLastError());
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		if (logFile)
		{
			fprintf_s(logFile, "[LoadUserFonts] exception: \"%s\"\n", e.what());
		}
	}
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == GetCurrentProcessId())
	{
		char title[500];
		GetWindowTextA(hwnd, title, sizeof(title));
		*(std::string*)lParam = title;
		return FALSE;
	}
	return TRUE;
}

bool IsProcessRunning(const char* processName) {
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	bool processFound = false;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		return false;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return false;
	}

	do {
		if (strcmp(pe32.szExeFile, processName) == 0) {
			processFound = true;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return processFound;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
#ifdef NDEBUG
	// Check if Yo2Launcher.exe is running
	if (!IsProcessRunning("Yo2Launcher.exe")) {
		MessageBoxA(NULL, "Please start the game over the Yo2Launcher.exe.", "Yu-Gi-Oh! Online: Duel Evolution", MB_OK | MB_ICONERROR);
		ExitProcess(1);
		return FALSE;
	}
#endif
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		MH_Initialize();
		DisableThreadLibraryCalls(hModule);
		HMODULE hExe = GetModuleHandle(NULL);

#if _DEBUG
		//MessageBoxW(0, L"Yu-GI-Oh DLL plugin mod runs in debug mode!", L"", 0);
#endif
		// ### MinHook
		// We can't be sure how the exe is named, so we scan memory to get accurate version strings
		std::string verYGOStr = ScanAndReadMemoryString(hExe, "Ver.0");
		std::string actualExe = getEXEPath();
		
		if (verYGOStr == "")
		{
			// PoC or YGO1 BETA
			// TODO
		}
		else
		{
			std::string verYGOStrExtra = ScanAndReadMemoryString(hExe, "release.20");
			if (verYGOStrExtra == "")
			{
				// YGO1
				YGO ygo = YGO(verYGOStr);
			}
			else
			{
				// YGO2
				int ygo2Version = getYGO2Version(actualExe);
				YGO2 ygo2 = YGO2(ygo2Version, verYGOStr);
			}
		}
		
		// Load winmm.dll stubs (the only DLL all yaneSDK-based games import)
		auto path = GetModuleFsPath(hModule);
		LoadDLL(path.filename());

		/*
		path = path.remove_filename();
		auto configPath = path / CONFIG_FILE;
		if (!fs::exists(configPath))
		{
			FILE* f;
			if (_wfopen_s(&f, configPath.c_str(), L"wb") == 0)
			{
				fputs(defConfigFile, f);
				fclose(f);
			}
		}

		GSOFontMode fixGSOFont = DISABLED;
		LOGFONTW userGSOFont = {};

		wchar_t errMsg[512];
		bool debug = true;

		// Seems not to be needed, dxvk fixes all gfx issues - maybe for bigger font mode
		// SetupYGOFontRendering();

		if (debug)
		{
			auto logPath = path / LOG_FILE;
			logFile = _wfsopen(logPath.c_str(), L"a+", _SH_DENYWR);
			setvbuf(logFile, nullptr, _IOLBF, BUFSIZ);
		}

		LoadUserFonts(path);

		switch (fixGSOFont)
		{
		case USE_NCM_FONT:
		{
			NONCLIENTMETRICSW ncm = { sizeof(ncm) };
			if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
			{
				newGSOFont = CreateFontIndirectW(&ncm.lfMessageFont);
				if (logFile)
				{
					std::string name;
					if (Utf16ToUtf8(ncm.lfMessageFont.lfFaceName, name))
					{
						fprintf_s(logFile, "[DllMain] SystemParametersInfo NONCLIENTMETRICS.lfMessageFont.lfFaceName=\"%s\"\n", name.c_str());
					}
				}
			}
			else if (logFile)
			{
				fprintf_s(logFile, "[DllMain] SystemParametersInfo failed. (%d)\n", GetLastError());
			}
		}
		break;
		case USE_USER_FONT:
		{
			newGSOFont = CreateFontIndirectW(&userGSOFont);
		}
		break;
		}

		auto hGdiFull = GetModuleHandleW(L"gdi32full.dll");
		if (hGdiFull)
		{
			auto addrGetStockObjectFull = reinterpret_cast<decltype(GetStockObject)*>(GetProcAddress(hGdiFull, "GetStockObject"));
			if (addrGetStockObjectFull)
				addrGetStockObject = addrGetStockObjectFull;

			auto addrCreateFontIndirectExWFull = reinterpret_cast<decltype(CreateFontIndirectExW)*>(GetProcAddress(hGdiFull, "CreateFontIndirectExW"));
			if (addrCreateFontIndirectExWFull)
				addrCreateFontIndirectExW = addrCreateFontIndirectExWFull;
		}

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		//if (fixGSOFont != DISABLED)
		//{
		//	DetourAttach(&(PVOID&)addrGetStockObject, MyGetStockObject);
		//}
		//DetourAttach(&(PVOID&)addrCreateFontIndirectExW, MyCreateFontIndirectExW);

		auto error = DetourTransactionCommit();
		if (error != ERROR_SUCCESS)
		{
			wchar_t msg[512];
			swprintf_s(msg, L"DetourTransactionCommit error: %d", error);

			SetThreadDpiAware();
			MessageBoxW(0, msg, L"Error", MB_ICONERROR);
			return TRUE;
		}
		*/
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		if (logFile)
			fclose(logFile);
	}
	return TRUE;
}
