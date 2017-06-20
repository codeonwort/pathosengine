#include "font_mgr.h"
#include <iostream>

namespace pathos {

	FontManager* FontManager::instance = nullptr;
	FontManager* FontManager::getInstance() {
		if (instance == nullptr) {
			instance = new FontManager;
		}
		return instance;
	}

	FontManager::FontManager() { }
	FontManager::~FontManager() { if (initialized) _term(); }

	bool FontManager::_init() {
		if (initialized) return true;
		if (FT_Init_FreeType(&library)) {
			std::cerr << "** Cannot init FreeType library **" << std::endl;
			return false;
		}

		initialized = true;
		return initialized;
	}

	bool FontManager::_term() {
		if (!initialized) return false;
		// Destroy all faces
		for (auto it = fontDB.begin(); it != fontDB.end(); ++it) {
			FaceMap* faceMap = it->second;
			for (auto it2 = faceMap->begin(); it2 != faceMap->end(); ++it2) {
				FT_Face face = it2->second;
				FT_Done_Face(face);
			}
		}
		// Release FreeType library
		if (FT_Done_FreeType(library)) {
			std::cerr << "** Error while destroying FreeType library **" << std::endl;
			return false;
		}
		return true;
	}

	bool FontManager::_loadFont(const std::string& tag, const char* name, unsigned int size) {
		FaceMap* map = new FaceMap;
		FT_Face face;
		std::string ary = " ./";
		for (char x = 'a'; x <= 'z'; x++) ary += x;
		for (char x = 'A'; x <= 'Z'; x++) ary += x;
		for (char x = '0'; x <= '9'; x++) ary += x;
		for (auto it = ary.begin(); it != ary.end(); it++) {
			char x = *it;
			if (_loadChar(&face, x, name, size) == false) {
				return false;
			}
			map->insert(std::pair<char, FT_Face>(x, face));
		}
		fontDB.insert(std::pair<std::string, FaceMap*>(tag, map));

		return true;
	}

	bool FontManager::_loadChar(FT_Face* face, char x, const char* name, unsigned int size) {
		if (FT_New_Face(library, name, 0, face)) {
			std::cerr << "** Cannot retrieve the font face" << std::endl;
			return false;
		}
		FT_Set_Pixel_Sizes(*face, 0, size);
		if (FT_Load_Char(*face, x, FT_LOAD_RENDER)) {
			std::cerr << "** Cannot load the character: " << x << std::endl;
			return false;
		}

		return true;
	}

	FaceMap* FontManager::_getFaceMap(const std::string& tag) {
		return fontDB.find(tag)->second;
	}

}