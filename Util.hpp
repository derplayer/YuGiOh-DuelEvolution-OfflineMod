#pragma once

// https://msdn.microsoft.com/en-us/magazine/mt763237
bool Utf8ToUtf16(const std::string_view& utf8, std::wstring& utf16)
{
	if (utf8.empty())
	{
		utf16.clear();
		return true;
	}

	constexpr DWORD kFlags = MB_ERR_INVALID_CHARS;

	const int utf8Length = static_cast<int>(utf8.length());

	const int utf16Length = MultiByteToWideChar(
		CP_UTF8,
		kFlags,
		utf8.data(),
		utf8Length,
		nullptr,
		0
	);

	if (utf16Length == 0)
	{
		return false;
	}

	utf16.resize(utf16Length);

	int result = MultiByteToWideChar(
		CP_UTF8,
		kFlags,
		utf8.data(),
		utf8Length,
		utf16.data(),
		utf16Length
	);

	if (result == 0)
	{
		return false;
	}

	return true;
}

bool Utf16ToUtf8(const std::wstring_view& utf16, std::string& utf8)
{
	if (utf16.empty())
	{
		utf8.clear();
		return true;
	}

	constexpr DWORD kFlags = WC_ERR_INVALID_CHARS;

	const int utf16Length = static_cast<int>(utf16.length());

	const int utf8Length = WideCharToMultiByte(
		CP_UTF8,
		kFlags,
		utf16.data(),
		utf16Length,
		nullptr,
		0,
		nullptr, nullptr
	);

	if (utf8Length == 0)
	{
		return false;
	}

	utf8.resize(utf8Length);

	int result = WideCharToMultiByte(
		CP_UTF8,
		kFlags,
		utf16.data(),
		utf16Length,
		utf8.data(),
		utf8Length,
		nullptr, nullptr
	);

	if (result == 0)
	{
		return false;
	}

	return true;
}

inline bool stol(const std::string& str, long& out)
{
	int& errno_ref = errno;
	const char *ptr = str.c_str();
	char *eptr;
	errno_ref = 0;
	out = strtol(ptr, &eptr, 10);

	if (ptr == eptr)
		return false;
	if (errno_ref == ERANGE)
		return false;
	return true;
}

inline bool stoul(const std::string& str, unsigned long& out)
{
	int& errno_ref = errno;
	const char *ptr = str.c_str();
	char *eptr;
	errno_ref = 0;
	out = strtoul(ptr, &eptr, 10);

	if (ptr == eptr)
		return false;
	if (errno_ref == ERANGE)
		return false;
	return true;
}

// https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/author-coclasses#add-helper-types-and-functions
auto GetModuleFsPath(HMODULE hModule)
{
	std::wstring path(MAX_PATH, L'\0');
	DWORD pathSize;
	DWORD actualSize;

	do
	{
		pathSize = static_cast<DWORD>(path.size());
		actualSize = GetModuleFileNameW(hModule, path.data(), pathSize);

		if (actualSize + 1 > pathSize)
		{
			path.resize(pathSize * 2);
		}
	} while (actualSize + 1 > pathSize);

	path.resize(actualSize);
	return fs::path(path);
}

void SetThreadDpiAware()
{
	HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
	if (hUser32)
	{
		using SetThreadDpiAwarenessContextPtr = DPI_AWARENESS_CONTEXT(WINAPI *)(DPI_AWARENESS_CONTEXT);
		auto funcPtr = reinterpret_cast<SetThreadDpiAwarenessContextPtr>(GetProcAddress(hUser32, "SetThreadDpiAwarenessContext"));
		if (funcPtr)
			funcPtr(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
	}
}

auto GetSysDirFsPath()
{
	std::wstring path(MAX_PATH, L'\0');
	while (1)
	{
		const UINT pathSize = static_cast<UINT>(path.size());
		const UINT requiredSize = GetSystemDirectoryW(path.data(), pathSize);
		path.resize(requiredSize);

		if (requiredSize < pathSize)
			break;
	}
	return fs::path(path);
}

// https://stackoverflow.com/a/4119881/6911112
bool iequals(std::wstring_view a, std::wstring_view b)
{
	return (a.size() == b.size()) && std::equal(a.begin(), a.end(), b.begin(), b.end(),
		[](wchar_t a, wchar_t b) { return a == b || tolower(a) == tolower(b); });
}

char* utf8cpy(char* dst, const char* src, size_t sizeDest)
{
	if (sizeDest) {
		size_t sizeSrc = strlen(src); // number of bytes not including null
		while (sizeSrc >= sizeDest) {

			const char* lastByte = src + sizeSrc; // Initially, pointing to the null terminator.
			while (lastByte-- > src)
				if ((*lastByte & 0xC0) != 0x80) // Found the initial byte of the (potentially) multi-byte character (or found null).
					break;

			sizeSrc = lastByte - src;
		}
		memcpy(dst, src, sizeSrc);
		dst[sizeSrc] = '\0';
	}
	return dst;
}

static int getYGO2Version(const char* filepath)
{
	DWORD dwDummyHandle, len;
	BYTE* buf = 0;
	unsigned int verlen;
	LPVOID lpvi;
	VS_FIXEDFILEINFO fileInfo;

	if (!filepath || !filepath[0]) return 0;

	len = GetFileVersionInfoSize((char*)filepath, &dwDummyHandle);
	if (!len) return 0;
	buf = (BYTE*)malloc(len * sizeof(BYTE));
	if (!buf) return 0;
	GetFileVersionInfo((char*)filepath, 0, len, buf);
	VerQueryValue(buf, "\\", &lpvi, &verlen);
	fileInfo = *(VS_FIXEDFILEINFO*)lpvi;
	free(buf);
	return fileInfo.dwProductVersionLS;
}