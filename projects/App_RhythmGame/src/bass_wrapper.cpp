#include "bass_wrapper.h"

#include "pathos/engine.h"
#include "pathos/util/log.h"
using namespace pathos;

#include "bass.h"

#pragma comment(lib, "bass.lib")

BassWrapper* gBass = nullptr;

bool BassWrapper::initializeBASS() {
	if (gBass != nullptr) {
		LOG(LogError, "[BASS] BASS is already initialized");
		return false;
	}

	gBass = new BassWrapper;
	
	bool bSuccess = gBass->initialize();
	if (bSuccess) {
		LOG(LogInfo, "[BASS] Initialize BASS");
	} else {
		LOG(LogError, "[BASS] Failed to initialize BASS");
	}

	return bSuccess;
}

bool BassWrapper::destroyBASS() {
	if (gBass == nullptr) {
		LOG(LogError, "[BASS] BASS is uninitialized or already destroyed");
		return false;
	}

	bool bSuccess = gBass->destroy();
	if (bSuccess) {
		LOG(LogInfo, "[BASS] Destroy BASS");
	} else {
		LOG(LogError, "[BASS] Failed to destroy BASS");
	}

	delete gBass;
	gBass = nullptr;

	return bSuccess;
}

bool BassWrapper::initialize() {
	return BASS_Init(
		-1,     // -1 = default device
		44100,  // freq
		0,      // flags
		0,      // hwnd,
		nullptr // clsid
	);
}

bool BassWrapper::destroy() {
	return BASS_Free();
}

bool BassWrapper::playFromFile(const char* filepath) {
	DWORD flags = 0;
	HSTREAM hstream = BASS_StreamCreateFile(
		false,
		filepath,
		0,
		0,
		flags
	);
	if (hstream == 0) {
		LOG(LogError, "[BASS] Failed to load: %s", filepath);
		return false;
	}
	if (BASS_ChannelStart(hstream) == false) {
		LOG(LogError, "[BASS] Failed to start: %s", filepath);
		return false;
	}

	return true;
}

bool BassWrapper::playFromFile(const wchar_t* wFilepath) {
	std::string filepath;
	WCHAR_TO_MBCS(wFilepath, filepath);
	return playFromFile(filepath.c_str());
}
