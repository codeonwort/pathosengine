#pragma once

namespace pathos {

	// Get text from OS clipboard.
	// @return true if successful. false if failed to access the clipboard for any reason.
	bool getOSClipboardUnicodeText(std::wstring& outText);

}
