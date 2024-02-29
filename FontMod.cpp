﻿#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include "Detours/src/detours.h"
#pragma comment(lib, "include/libMinHook-x86-v141-mt.lib")
//#pragma comment(lib, "include/MinHook.x86.dll")
#include "include/MinHook.h"
#include "Util.hpp"
#include "dllstub.hpp"
#include "DefConfigFile.hpp"

#include "YGO2.h"

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

bool LoadSettings(HMODULE hModule, const fs::path& fileName, wchar_t* errMsg, GSOFontMode& fixGSOFont, LOGFONT& userGSOFont, bool& debug)
{
	bool ret = false;
	std::ifstream fin(fileName);
	if (fin)
	{
		//do {
		//	YAML::Node config;
		//	try
		//	{
		//		config = YAML::Load(fin);
		//	}
		//	catch (const std::exception& e)
		//	{
		//		swprintf(errMsg, 512, L"YAML::Load error.\n%hs", e.what());
		//		break;
		//	}

		//	if (!config.IsMap())
		//	{
		//		swprintf(errMsg, 512, L"Root node is not a map.");
		//		break;
		//	}

		//	if (auto node = FindNode(config, "fonts"); node && node.IsMap())
		//	{
		//		for (const auto& i : node)
		//		{
		//			if (i.first.IsScalar() && i.second.IsMap())
		//			{
		//				YAML::Node replace;
		//				if (auto r = FindNode(i.second, "replace"); r && r.IsScalar())
		//				{
		//					replace = r;
		//				}
		//				else
		//				{
		//					replace = FindNode(i.second, "name");
		//				}
		//				if (replace && replace.IsScalar())
		//				{
		//					font fontInfo;
		//					Utf8ToUtf16(replace.as<std::string>(), fontInfo.replace);
		//					fontInfo.overrideFlags = _NONE;

		//					if (auto node = FindNode(i.second, "size"); node && node.IsScalar())
		//					{
		//						if (stol(node.as<std::string>(), fontInfo.height))
		//							fontInfo.overrideFlags |= _HEIGHT;
		//					}

		//					if (auto node = FindNode(i.second, "width"); node && node.IsScalar())
		//					{
		//						if (stol(node.as<std::string>(), fontInfo.width))
		//							fontInfo.overrideFlags |= _WIDTH;
		//					}

		//					if (auto node = FindNode(i.second, "weight"); node && node.IsScalar())
		//					{
		//						if (stol(node.as<std::string>(), fontInfo.weight))
		//							fontInfo.overrideFlags |= _WEIGHT;
		//					}

		//					if (auto node = FindNode(i.second, "italic"); node && node.IsScalar())
		//					{
		//						fontInfo.overrideFlags |= _ITALIC;
		//						fontInfo.italic = node.as<bool>();
		//					}

		//					if (auto node = FindNode(i.second, "underLine"); node && node.IsScalar())
		//					{
		//						fontInfo.overrideFlags |= _UNDERLINE;
		//						fontInfo.underLine = node.as<bool>();
		//					}

		//					if (auto node = FindNode(i.second, "strikeOut"); node && node.IsScalar())
		//					{
		//						fontInfo.overrideFlags |= _STRIKEOUT;
		//						fontInfo.strikeOut = node.as<bool>();
		//					}

		//					if (auto node = FindNode(i.second, "charSet"); node && node.IsScalar())
		//					{
		//						unsigned long out;
		//						if (stoul(node.as<std::string>(), out))
		//						{
		//							fontInfo.overrideFlags |= _CHARSET;
		//							fontInfo.charSet = static_cast<BYTE>(out);
		//						}
		//					}

		//					if (auto node = FindNode(i.second, "outPrecision"); node && node.IsScalar())
		//					{
		//						unsigned long out;
		//						if (stoul(node.as<std::string>(), out))
		//						{
		//							fontInfo.overrideFlags |= _OUTPRECISION;
		//							fontInfo.outPrecision = static_cast<BYTE>(out);
		//						}
		//					}

		//					if (auto node = FindNode(i.second, "clipPrecision"); node && node.IsScalar())
		//					{
		//						unsigned long out;
		//						if (stoul(node.as<std::string>(), out))
		//						{
		//							fontInfo.overrideFlags |= _CLIPPRECISION;
		//							fontInfo.clipPrecision = static_cast<BYTE>(out);
		//						}
		//					}

		//					if (auto node = FindNode(i.second, "quality"); node && node.IsScalar())
		//					{
		//						unsigned long out;
		//						if (stoul(node.as<std::string>(), out))
		//						{
		//							fontInfo.overrideFlags |= _QUALITY;
		//							fontInfo.quality = static_cast<BYTE>(out);
		//						}
		//					}

		//					if (auto node = FindNode(i.second, "pitchAndFamily"); node && node.IsScalar())
		//					{
		//						unsigned long out;
		//						if (stoul(node.as<std::string>(), out))
		//						{
		//							fontInfo.overrideFlags |= _PITCHANDFAMILY;
		//							fontInfo.pitchAndFamily = static_cast<BYTE>(out);
		//						}
		//					}

		//					std::wstring find;
		//					Utf8ToUtf16(i.first.as<std::string>(), find);
		//					fontsMap[find] = fontInfo;
		//				}
		//			}
		//		}
		//	}

		//	if (auto node = FindNode(config, "fixGSOFont"); node)
		//	{
		//		if (node.IsScalar())
		//		{
		//			if (node.as<bool>())
		//				fixGSOFont = USE_NCM_FONT;
		//		}
		//		else if (node.IsMap())
		//		{
		//			YAML::Node name;
		//			if (auto r = FindNode(node, "replace"); r && r.IsScalar())
		//			{
		//				name = r;
		//			}
		//			else
		//			{
		//				name = FindNode(node, "name");
		//			}
		//			if (name && name.IsScalar())
		//			{
		//				fixGSOFont = USE_USER_FONT;

		//				std::wstring faceName;
		//				Utf8ToUtf16(name.as<std::string>(), faceName);
		//				memcpy_s(userGSOFont.lfFaceName, sizeof(userGSOFont.lfFaceName), faceName.c_str(), faceName.size() * sizeof(decltype(faceName)::value_type));

		//				if (auto n = FindNode(node, "size"); n && n.IsScalar())
		//				{
		//					stol(n.as<std::string>(), userGSOFont.lfHeight);
		//				}

		//				if (auto n = FindNode(node, "width"); n && n.IsScalar())
		//				{
		//					stol(n.as<std::string>(), userGSOFont.lfWidth);
		//				}

		//				if (auto n = FindNode(node, "weight"); n && n.IsScalar())
		//				{
		//					stol(n.as<std::string>(), userGSOFont.lfWeight);
		//				}

		//				if (auto n = FindNode(node, "italic"); n && n.IsScalar())
		//				{
		//					userGSOFont.lfItalic = n.as<bool>();
		//				}

		//				if (auto n = FindNode(node, "underLine"); n && n.IsScalar())
		//				{
		//					userGSOFont.lfUnderline = n.as<bool>();
		//				}

		//				if (auto n = FindNode(node, "strikeOut"); n && n.IsScalar())
		//				{
		//					userGSOFont.lfStrikeOut = n.as<bool>();
		//				}

		//				if (auto n = FindNode(node, "charSet"); n && n.IsScalar())
		//				{
		//					unsigned long out;
		//					stoul(n.as<std::string>(), out);
		//					userGSOFont.lfCharSet = static_cast<BYTE>(out);
		//				}

		//				if (auto n = FindNode(node, "outPrecision"); n && n.IsScalar())
		//				{
		//					unsigned long out;
		//					stoul(n.as<std::string>(), out);
		//					userGSOFont.lfOutPrecision = static_cast<BYTE>(out);
		//				}

		//				if (auto n = FindNode(node, "clipPrecision"); n && n.IsScalar())
		//				{
		//					unsigned long out;
		//					stoul(n.as<std::string>(), out);
		//					userGSOFont.lfClipPrecision = static_cast<BYTE>(out);
		//				}

		//				if (auto n = FindNode(node, "quality"); n && n.IsScalar())
		//				{
		//					unsigned long out;
		//					stoul(n.as<std::string>(), out);
		//					userGSOFont.lfQuality = static_cast<BYTE>(out);
		//				}

		//				if (auto n = FindNode(node, "pitchAndFamily"); n && n.IsScalar())
		//				{
		//					unsigned long out;
		//					stoul(n.as<std::string>(), out);
		//					userGSOFont.lfPitchAndFamily = static_cast<BYTE>(out);
		//				}
		//			}
		//		}
		//	}

		//	if (auto node = FindNode(config, "debug"); node && node.IsScalar())
		//		debug = node.as<bool>();

		//	ret = true;
		//} while (0);
	}
	else
	{
#pragma warning(push)
#pragma warning(disable: 4996) // 'strerror': This function or variable may be unsafe.
		swprintf(errMsg, 512, L"Can not open " CONFIG_FILE ".\n%hs", strerror(errno));
#pragma warning(pop)
	}
	return ret;
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

DWORD WINAPI HookThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		//StartHooks();
		init_hook = true;
	} while (!init_hook);
	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		MH_Initialize();
		DisableThreadLibraryCalls(hModule);

#if _DEBUG
		//MessageBoxW(0, L"Yu-GI-Oh DLL plugin mod runs in debug mode!", L"", 0);
#endif

		// ### MinHook
		CreateThread(nullptr, 0, HookThread, hModule, 0, nullptr);

		// detect game version
		int ygo2Version = getYGO2Version("yo2.exe");
		YGO2 ygo2 = YGO2(ygo2Version);

		//int ygoVersion = getYGO2Version("online_pc.exe");

		// ### FontMod
		auto path = GetModuleFsPath(hModule);
		LoadDLL(path.filename());

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
		//if (!LoadSettings(hModule, configPath, errMsg, fixGSOFont, userGSOFont, debug))
		//{
		//	wchar_t msg[512];
		//	swprintf_s(msg, L"LoadSettings error.\n%s", errMsg);

		//	SetThreadDpiAware();
		//	MessageBoxW(0, msg, L"Error", MB_ICONERROR);
		//	return TRUE;
		//}

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

		if (fixGSOFont != DISABLED)
		{
			DetourAttach(&(PVOID&)addrGetStockObject, MyGetStockObject);
		}
		DetourAttach(&(PVOID&)addrCreateFontIndirectExW, MyCreateFontIndirectExW);

		auto error = DetourTransactionCommit();
		if (error != ERROR_SUCCESS)
		{
			wchar_t msg[512];
			swprintf_s(msg, L"DetourTransactionCommit error: %d", error);

			SetThreadDpiAware();
			MessageBoxW(0, msg, L"Error", MB_ICONERROR);
			return TRUE;
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		if (logFile)
			fclose(logFile);
	}
	return TRUE;
}
