#pragma once

#include <string>

namespace pathos {

	// Path of the exectuable
	void getExecPath(std::wstring& outPath);

	// Directory that contains the exectuable
	void getExecDir(std::wstring& outDir);

	// Returns absolute path.
	std::string getAbsolutePath(const char* targetDir);

	// Create the directory if not exist.
	void createDirectory(const char* targetDir);

	// Returns directory that contains 'PathosEngine.sln'.
	std::string getSolutionDir();

}
