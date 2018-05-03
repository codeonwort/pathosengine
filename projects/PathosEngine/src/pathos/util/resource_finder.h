#pragma once

#include <vector>
#include <string>

namespace pathos {

	// Given a path, searches for a matching full path.
	class ResourceFinder {
	public:
		static ResourceFinder& get();
		// register a directory
		void add(const std::string& directory);
		// Returns empty string if not found
		std::string find(const std::string& subpath);
	private:
		ResourceFinder();
		ResourceFinder(const ResourceFinder& other) = delete;
		ResourceFinder(ResourceFinder&& rhs) = delete;
		std::vector<std::string> directories;
	};

}