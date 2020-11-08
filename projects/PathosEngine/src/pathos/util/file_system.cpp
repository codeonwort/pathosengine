#include "file_system.h"

// #todo: cross-platform way (std::filesystem in C++17)
#include <Windows.h>
#include <Shlwapi.h>

namespace pathos {

	void getExecPath(std::wstring& outPath)
	{
		wchar_t buffer[1024];
		DWORD length = ::GetModuleFileName(NULL, buffer, sizeof(buffer));
		outPath.assign(buffer, length);
	}

	void getExecDir(std::wstring& outDir)
	{
		std::wstring path;
		getExecPath(path);
		size_t ix = path.find_last_of(L'\\');
		outDir = path.substr(0, ix + 1);
	}

	std::string getFullDirectoryPath(const char* targetDir)
	{
		char buffer[1024];
		if (::GetFullPathNameA(targetDir, 1024, buffer, NULL) != 0) {
			return std::string(buffer);
		} else {
			return "";
		}
	}

	void createDirectory(const char* targetDir)
	{
		::CreateDirectoryA(targetDir, NULL);
	}

}
