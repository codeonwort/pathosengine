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

	std::string getAbsolutePath(const char* targetDir)
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

	bool pathExists(const char* path) {
#if PLATFORM_WINDOWS
		return (TRUE == ::PathFileExistsA(path));
#else
		#error "Not implemented"
#endif
	}

	bool enumerateFiles(const char* targetDir, std::vector<std::string>& outFilepaths) {
		if (pathExists(targetDir) == false) {
			return false;
		}

		outFilepaths.clear();

#if PLATFORM_WINDOWS
		WIN32_FIND_DATAA findData;
		HANDLE hFind;
		char targetDirEx[1024];
		sprintf_s(targetDirEx, "%s\\*", targetDir);
		hFind = ::FindFirstFileA(targetDirEx, &findData);
		do {
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// #todo-filesystem: Support recursive enumeration?
			} else {
				outFilepaths.push_back(findData.cFileName);
			}
		} while (::FindNextFileA(hFind, &findData) != FALSE);
		::FindClose(hFind);
#else
		#error "Not implemented"
#endif
		return true;
	}

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

	std::string getSolutionDir()
	{
		std::string solutionPath = ResourceFinder::get().find("PathosEngine.sln");
		std::string solutionDir;
		if (solutionPath.size() > 0) {
			solutionDir = solutionPath.substr(0, solutionPath.size() - std::string("PathosEngine.sln").size());
		}
		CHECKF(solutionDir.size() != 0, "Maybe the solution name has been changed?");
		solutionDir = getAbsolutePath(solutionDir.c_str());
		return solutionDir;
	}

}
