#include "resource_finder.h"
#include <assert.h>

////////////////////////////////////////////////////////
// Platform-specific
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
static inline bool fileExists(const std::string& path) {
	return PathFileExistsA(path.c_str()) == TRUE ? true : false;
}
////////////////////////////////////////////////////////

namespace pathos {

	ResourceFinder& ResourceFinder::get() {
		// C++11 guarantees this is thread-safe
		static ResourceFinder instance;
		return instance;
	}

	ResourceFinder::ResourceFinder() {
		directories.push_back("./");
	}

	void ResourceFinder::add(const std::string& directory) {
		auto last = directory.at(directory.size() - 1);
		assert(last == '/' || last == '\\');
		directories.push_back(directory);
	}

	std::string ResourceFinder::find(const std::string& subpath) {
		for (const auto& dir : directories) {
			auto fullpath = dir + subpath;
			if (fileExists(fullpath)) {
				return fullpath;
			}
		}
		return "";
	}

}