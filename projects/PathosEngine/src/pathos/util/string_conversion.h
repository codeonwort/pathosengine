#pragma once

#include <string>

namespace pathos {

	void MBCS_TO_WCHAR(const std::string& inStr, std::wstring& outStr);

	void WCHAR_TO_MBCS(const std::wstring& inStr, std::string& outStr);

}
