#pragma once

class BassWrapper {

public:
	static bool initializeBASS();
	static bool destroyBASS();

private:
	bool initialize();
	bool destroy();

public:
	bool playFromFile(const char* filepath, float volume = 1.0f);
	bool playFromFile(const wchar_t* wFilepath, float volume = 1.0f);

};

extern BassWrapper* gBass;
