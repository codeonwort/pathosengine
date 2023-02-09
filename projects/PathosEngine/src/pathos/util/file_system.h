#pragma once

#include <string>

// This was written before adopting C++17, so most of them are obsolete now.
namespace pathos {

	// --------------------------------------------------------
	// Generic filesystem functions

	// Returns absolute path.
	std::string getAbsolutePath(const char* targetDir);

	// @return Parent directory path of the file. Empty string if invalid.
	std::string getDirectoryPath(const char* filePath);

	// Create the directory if not exist.
	void createDirectory(const char* targetDir);

	bool pathExists(const char* path);

	// Find files in the given directory.
	// Returns false if the directory does not exist.
	bool enumerateFiles(const char* targetDir, std::vector<std::string>& outFilepaths);

	// --------------------------------------------------------
	// Project-specific utils

	// Path of the exectuable
	void getExecPath(std::wstring& outPath);

	// Directory that contains the exectuable
	void getExecDir(std::wstring& outDir);

	// Returns directory that contains 'PathosEngine.sln'.
	std::string getSolutionDir();

}
