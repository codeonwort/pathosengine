#include "string_conversion.h"

#include <vector>
#include <Windows.h>

namespace pathos {

	void MBCS_TO_WCHAR(const std::string& inStr, std::wstring& outStr)
	{
		int size = ::MultiByteToWideChar(CP_ACP, 0, inStr.data(), (int)inStr.size(), NULL, 0);
		outStr.assign(size, 0);
		::MultiByteToWideChar(CP_ACP, 0, inStr.data(), (int)inStr.size(), const_cast<wchar_t*>(outStr.data()), size);
	}

	void WCHAR_TO_MBCS(const std::wstring& inStr, std::string& outStr)
	{
		int size = ::WideCharToMultiByte(CP_ACP, 0, inStr.data(), (int)inStr.size(), NULL, 0, NULL, NULL);
		outStr.assign(size, 0);
		::WideCharToMultiByte(CP_ACP, 0, inStr.data(), (int)inStr.size(), const_cast<char*>(outStr.data()), size, NULL, NULL);
	}

}
