#include "file_system.h"
#include "resource_finder.h"

#include "badger/system/platform.h"
#include "badger/assertion/assertion.h"

#include <filesystem>

// For getExecPath()
#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace pathos {

	std::string getAbsolutePath(const char* targetDir) {
		if (std::filesystem::exists(targetDir)) {
			return std::filesystem::absolute(targetDir).string();
		}
		return "";
	}

	std::string getDirectoryPath(const char* filePath) {
		std::filesystem::path stdPath = filePath;
		stdPath.remove_filename();
		return stdPath.string();
	}

	void createDirectory(const char* targetDir) {
		std::filesystem::create_directories(targetDir);
	}

	bool pathExists(const char* path) {
		return std::filesystem::exists(path);
	}

	static void enumerateFilesSub(std::filesystem::path dir, bool recursive, std::vector<std::string>& outFilepaths) {
		for (const auto& dir_entry : std::filesystem::directory_iterator(dir)) {
			if (recursive && dir_entry.is_directory()) {
				enumerateFilesSub(dir_entry.path(), recursive, outFilepaths);
			} else {
				outFilepaths.push_back(dir_entry.path().filename().string());
			}
		}
	}
	bool enumerateFiles(const char* targetDir, bool recursive, std::vector<std::string>& outFilepaths) {
		if (std::filesystem::is_directory(targetDir) == false) {
			return false;
		}
		outFilepaths.clear();
		enumerateFilesSub(targetDir, recursive, outFilepaths);
		return true;
	}

	void getExecPath(std::wstring& outPath) {
#if PLATFORM_WINDOWS
		wchar_t buffer[1024];
		DWORD length = ::GetModuleFileName(NULL, buffer, sizeof(buffer));
		outPath.assign(buffer, length);
#else
		#error "Not implemented"
#endif
	}

	void getExecDir(std::wstring& outDir) {
		std::wstring path;
		getExecPath(path);
		size_t ix = path.find_last_of(L'\\');
		outDir = path.substr(0, ix + 1);
	}

	std::string getSolutionDir() {
		std::string solutionPath = ResourceFinder::get().find("PathosEngine.sln");
		std::string solutionDir;
		if (solutionPath.size() > 0) {
			solutionDir = solutionPath.substr(0, solutionPath.size() - std::string("PathosEngine.sln").size());
		}
		CHECKF(solutionDir.size() != 0, "Couldn't find PathosEngine.sln");
		solutionDir = getAbsolutePath(solutionDir.c_str());
		return solutionDir;
	}

}
