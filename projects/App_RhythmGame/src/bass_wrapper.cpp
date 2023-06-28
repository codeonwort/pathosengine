#include "bass_wrapper.h"

#include "pathos/engine.h"
#include "pathos/util/log.h"
using namespace pathos;

#include "bass.h"

#pragma comment(lib, "bass.lib")

//////////////////////////////////////////////////////////////////////////
// BassWrapper

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

BassStream* BassWrapper::createStreamFromFile(const char* filepath, float volume) {
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
		return nullptr;
	}

	//BASS_CHANNELINFO info;
	//BASS_ChannelGetInfo(hstream, &info);

	BASS_ChannelSetAttribute(hstream, BASS_ATTRIB_VOLDSP, volume);

	return new BassStream(hstream, filepath);
}

BassStream* BassWrapper::createStreamFromFile(const wchar_t* wFilepath, float volume) {
	std::string filepath;
	WCHAR_TO_MBCS(wFilepath, filepath);
	return createStreamFromFile(filepath.c_str(), volume);
}

//////////////////////////////////////////////////////////////////////////
// BassStream

void BassStream::startPlay() {
	DWORD hstream = (DWORD)handle;
	if (BASS_ChannelStart(hstream) == false) {
		LOG(LogError, "[BASS] Failed to start: %s", source.c_str());
	}
}

void BassStream::setVolume(float vol) {
	if (handle != 0) {
		DWORD hstream = (DWORD)handle;
		BASS_ChannelSetAttribute(hstream, BASS_ATTRIB_VOLDSP, vol);
	}
}
