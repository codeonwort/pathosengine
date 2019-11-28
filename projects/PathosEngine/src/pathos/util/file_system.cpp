#include "file_system.h"

// #todo: cross-platform way
#include <Windows.h>

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

}
