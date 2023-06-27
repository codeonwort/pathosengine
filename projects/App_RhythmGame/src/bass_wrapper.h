#pragma once

#include "badger/types/int_types.h"
#include <string>

class BassStream {

public:
	BassStream(uint64 inHandle, const char* inSource)
		: handle(inHandle)
		, source(inSource)
	{
	}

	void startPlay();

private:
	uint64 handle = 0; // HSTREAM
	std::string source;
};

class BassWrapper {

public:
	static bool initializeBASS();
	static bool destroyBASS();

private:
	bool initialize();
	bool destroy();

public:
	BassStream* createStreamFromFile(const char* filepath, float volume = 1.0f);
	BassStream* createStreamFromFile(const wchar_t* wFilepath, float volume = 1.0f);

};

extern BassWrapper* gBass;
