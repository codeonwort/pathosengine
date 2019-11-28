#pragma once

#include <string>

namespace pathos {

	// Path of the exectuable
	void getExecPath(std::wstring& outPath);

	// Directory that contains the exectuable
	void getExecDir(std::wstring& outDir);

}
