#pragma once

class BassWrapper {

public:
	static bool initializeBASS();
	static bool destroyBASS();

private:
	bool initialize();
	bool destroy();

public:
	bool playFromFile(const char* filepath);
	bool playFromFile(const wchar_t* wFilepath);

};

extern BassWrapper* gBass;
