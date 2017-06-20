#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <string>

namespace pathos {

	using FaceMap = std::map<char, FT_Face>;
	using FontDB = std::map<std::string, FaceMap*>;

	class FontManager {

	// static
	public:
		static bool init() { return getInstance()->_init(); }
		static bool term() { return getInstance()->_term(); }
		static bool available() { return getInstance()->initialized; }
		static bool loadFont(const std::string& tag, const char* name, unsigned int size) { return getInstance()->_loadFont(tag, name, size); }
		static FaceMap* getFaceMap(const std::string& tag) { return getInstance()->_getFaceMap(tag); }
	private:
		static FontManager* instance;
		static FontManager* getInstance();

	// non-static
	public:
		FontManager();
		FontManager(const FontManager& other) = delete;
		FontManager(FontManager&& rhs) = delete;
		~FontManager();
	private:
		bool initialized = false;
		FT_Library library;
		FontDB fontDB;

		bool _init();
		bool _term();
		bool _loadFont(const std::string& tag, const char* name, unsigned int size);
		bool _loadChar(FT_Face* face, char x, const char* name, unsigned int size);
		FaceMap* _getFaceMap(const std::string& tag);
	};

}