#include "os_util.h"

#include "badger/system/platform.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
// https://stackoverflow.com/questions/14762456/getclipboarddatacf-text
static std::wstring getWin32ClipboardText() {
	if (!::OpenClipboard(nullptr)) {
		return L"";
	}
	HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
	if (hData == NULL) {
		return L"";
	}
	wchar_t* pszText = static_cast<wchar_t*>(::GlobalLock(hData));
	if (pszText == NULL) {
		return L"";
	}
	std::wstring msg(pszText);
	::GlobalUnlock(hData);
	::CloseClipboard();
	return msg;
}
#endif

namespace pathos {

	bool getOSClipboardUnicodeText(std::wstring& outText) {
#if PLATFORM_WINDOWS
		outText = getWin32ClipboardText();
		return outText.size() > 0;
#else
		#error Not implemented.
		return false;
#endif
	}

}
