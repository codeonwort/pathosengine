#include "file_system.h"
#include "resource_finder.h"

#include "badger/system/platform.h"
#include "badger/assertion/assertion.h"

#if PLATFORM_WINDOWS
// #todo: cross-platform way (std::filesystem in C++17)
#include <Windows.h>
#include <Shlwapi.h>
#endif

namespace pathos {

	void getExecPath(std::wstring& outPath)
	{
#if PLATFORM_WINDOWS
		wchar_t buffer[1024];
		DWORD length = ::GetModuleFileName(NULL, buffer, sizeof(buffer));
		outPath.assign(buffer, length);
#else
		#error "Not implemented"
#endif
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
#if PLATFORM_WINDOWS
		char buffer[1024];
		if (::GetFullPathNameA(targetDir, 1024, buffer, NULL) != 0) {
			return std::string(buffer);
		} else {
			return "";
		}
#else
		#error "Not implemented"
#endif
	}

	void createDirectory(const char* targetDir)
	{
#if PLATFORM_WINDOWS
		::CreateDirectoryA(targetDir, NULL);
#else
		#error "Not implemented"
#endif
	}

	std::string getSolutionDir()
	{
		std::string solutionPath = ResourceFinder::get().find("PathosEngine.sln");
		std::string solutionDir;
		if (solutionPath.size() > 0) {
			solutionDir = solutionPath.substr(0, solutionPath.size() - std::string("PathosEngine.sln").size());
		}
		CHECKF(solutionDir.size() != 0, "Maybe the solution name has been changed?");
		solutionDir = getFullDirectoryPath(solutionDir.c_str());
		return solutionDir;
	}

}
