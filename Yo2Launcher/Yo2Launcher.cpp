// Yo2Launcher.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <windows.h>
#include <winhttp.h>
#include <json.hpp>
#include <modversion.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "user32.lib")

// Define function pointers for TitanEngine functions
typedef void* (*InitDebug_t)(char*, char*, char*);
typedef BOOL(*AttachDebugger_t)(DWORD);
typedef BOOL(*DebugLoop_t)(void*);
typedef void (*SetCustomHandler_t)(void*);

void AttachTitanDebugger() {
	HMODULE hTitanEngine = LoadLibrary(L"TitanEngine.dll");
	if (!hTitanEngine) {
		MessageBox(NULL, L"Failed to load TitanEngine.dll", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Get the function addresses
	InitDebug_t InitDebug = (InitDebug_t)GetProcAddress(hTitanEngine, "InitDebug");
	AttachDebugger_t AttachDebugger = (AttachDebugger_t)GetProcAddress(hTitanEngine, "AttachDebugger");
	DebugLoop_t DebugLoop = (DebugLoop_t)GetProcAddress(hTitanEngine, "DebugLoop");
	SetCustomHandler_t SetCustomHandler = (SetCustomHandler_t)GetProcAddress(hTitanEngine, "SetCustomHandler");

	if (!InitDebug || !AttachDebugger || !DebugLoop || !SetCustomHandler) {
		MessageBox(NULL, L"Failed to get function addresses from TitanEngine.dll", L"Error", MB_OK | MB_ICONERROR);
		FreeLibrary(hTitanEngine);
		return;
	}

	// Get the full path to the executable dynamically
	char szFileName[MAX_PATH];
	if (!GetModuleFileNameA(NULL, szFileName, MAX_PATH)) {
		MessageBoxA(NULL, "Failed to get the module file name", "Error", MB_OK | MB_ICONERROR);
		FreeLibrary(hTitanEngine);
		return;
	}

	// Set the current folder
	char szCurrentFolder[MAX_PATH];
	strcpy_s(szCurrentFolder, MAX_PATH, szFileName);
	*strrchr(szCurrentFolder, '\\') = '\0';  // Remove the executable name to get the folder path

	// Manually set the target executable name
	char targetExe[MAX_PATH];
	sprintf_s(targetExe, MAX_PATH, "%s\\yo2.exe", szCurrentFolder);

	char szCommandLine[] = "";  // Command line parameters, if any
	PROCESS_INFORMATION* pi = (PROCESS_INFORMATION*)InitDebug(targetExe, szCommandLine, szCurrentFolder);
	if (!pi) {
		MessageBoxA(NULL, "Failed to initialize debugging with TitanEngine", "Error", MB_OK | MB_ICONERROR);
		FreeLibrary(hTitanEngine);
		return;
	}

	DWORD processId = pi->dwProcessId;

	// Optionally set a custom exception handler
	// SetCustomHandler(NULL);

	// Start the debug loop
	if (!DebugLoop(NULL)) {
		MessageBox(NULL, L"Failed to start debug loop with TitanEngine", L"Error", MB_OK | MB_ICONERROR);
	}

	FreeLibrary(hTitanEngine);
}

void ShowErrorMessage(const char* message) {
	MessageBoxA(NULL, message, "Yo2Launcher error", MB_OK | MB_ICONERROR);
}

std::string LoadJsonFromUrl(const std::wstring& url) {
	std::wstring server(L"");
	std::wstring uri(L"/");
	URL_COMPONENTSW urlComp = { sizeof(urlComp) };
	urlComp.dwHostNameLength = (DWORD)-1;
	urlComp.dwUrlPathLength = (DWORD)-1;
	urlComp.dwExtraInfoLength = (DWORD)-1;
	WinHttpCrackUrl(url.c_str(), 0, 0, &urlComp);

	server.assign(urlComp.lpszHostName, urlComp.dwHostNameLength);
	uri.assign(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
	if (urlComp.lpszExtraInfo) {
		uri.append(urlComp.lpszExtraInfo);
	}

	HINTERNET hSession = WinHttpOpen(L"YGO2 Mod Client/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		ShowErrorMessage("Connection couldn't be established.");
		return "";
	}

	HINTERNET hConnect = WinHttpConnect(hSession, server.c_str(), urlComp.nPort, 0);
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		ShowErrorMessage("Connection couldn't be established.");
		return "";
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", uri.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		ShowErrorMessage("Connection couldn't be established.");
		return "";
	}

	DWORD timeout = 3000; // Timeout in milliseconds
	WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
	WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
	WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));

	BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	if (!bResults) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		ShowErrorMessage("Connection couldn't be established.");
		return "";
	}

	bResults = WinHttpReceiveResponse(hRequest, NULL);
	if (!bResults) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		ShowErrorMessage("Connection couldn't be established.");
		return "";
	}

	std::string jsonStr;
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;

	do {
		dwSize = 0;
		if (WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			pszOutBuffer = new char[dwSize + 1];
			if (pszOutBuffer) {
				ZeroMemory(pszOutBuffer, dwSize + 1);
				if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
					jsonStr.append(pszOutBuffer, dwDownloaded);
				}
				delete[] pszOutBuffer;
			}
		}
	} while (dwSize > 0);

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return jsonStr;
}

bool CheckVersionAndUpdate(const std::string& jsonStr) {
	try
	{
		nlohmann::json jsonData = nlohmann::json::parse(jsonStr);
		double newestVersion = jsonData["newestVersion"].get<double>();
		std::string updateUrl = jsonData["url"].get<std::string>();
		std::string updateUrlFile = jsonData["urlFile"].get<std::string>();
		std::string message = jsonData["message"].get<std::string>();

		double currentVersion = YGO2_MODVERSION / 100.0; // load modversion from shared include

		if (newestVersion > currentVersion) {
			std::string prompt = "A new offline mod version (" + std::to_string(newestVersion) + ") is available. Do you want to open the update page?\n\n# Server message #\n" + message;
			if (MessageBoxA(NULL, prompt.c_str(), "Update Available", MB_YESNO | MB_ICONQUESTION) == IDYES) {
				ShellExecuteA(NULL, "open", updateUrl.c_str(), NULL, NULL, SW_SHOWNORMAL); // Opens the update file URL
				return true;
			}
		}

		return false;
	}
	catch (const nlohmann::json::parse_error& e) {
		ShowErrorMessage("Failed to parse JSON. The response from the mod update server is not valid JSON.");
		return false;
	}
	catch (const std::exception& e) {
		ShowErrorMessage(e.what());
		return false;
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	std::wstring url = L"https://derplayer.neocities.org/repo/yugioh/versionDuelEvo.json";
	std::string jsonStr = LoadJsonFromUrl(url);
	bool res = CheckVersionAndUpdate(jsonStr);
	if (res == true) return 0;

	AttachTitanDebugger();
	std::cout << "Yu-Gi-Oh 2 debugger enforced! You shouldn't see this message! If you do, then well... it is what it is.\n";
	return 0;
}
