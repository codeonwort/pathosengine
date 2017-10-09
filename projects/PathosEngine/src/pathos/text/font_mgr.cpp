#include "font_mgr.h"
#include <iostream>

namespace pathos {

	///////////////////////////////////////////////////////////////////////////////////////////
	// static
	FontManager* FontManager::instance = nullptr;
	FontManager* FontManager::getInstance() {
		if (instance == nullptr) {
			instance = new FontManager;
		}
		return instance;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// non-static
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
		// Destroy all glyphs
		for (auto it = fontDB.begin(); it != fontDB.end(); ++it) {
			GlyphMap* glyphs = it->second;
			auto& mapping = glyphs->mapping;
			for (auto it2 = mapping.begin(); it2 != mapping.end(); ++it2) {
				auto buffer = it2->second.bitmap.buffer;
				delete[] buffer;
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
		FT_Face face;
		std::wstring ary = L" ";
		for (wchar_t x = 33; x <= 126; ++x) ary += x;

		if (FT_New_Face(library, name, 0, &face)) {
			std::cerr << "** Cannot retrieve the font face" << std::endl;
			return false;
		}
		if (FT_Set_Pixel_Sizes(face, 0, size)) {
			std::cerr << "** Cannot set pixel size" << std::endl;
			return false;
		}

		GlyphMap* map = new GlyphMap;
		map->filename = name;
		map->fontSize = size;
		map->maxHeight = face->size->metrics.height >> 6;

		for (auto it = ary.begin(); it != ary.end(); ++it) {
			wchar_t x = *it;
			if (_loadChar(face, x, map->mapping) == false) {
				return false;
			}
		}
		fontDB[tag] = map;

		FT_Done_Face(face);

		return true;
	}

	bool FontManager::_loadChar(FT_Face face, wchar_t x, std::map<wchar_t, FT_GlyphCache>& mapping) {
		if (FT_Load_Char(face, x, FT_LOAD_RENDER)) {
			std::cerr << "** Cannot load the character: " << x << std::endl;
			return false;
		}
		// Each time FT_Load_Char() is called, face's glyph slot is replaced with new information.
		// We need to cache the glyphs.
		FT_GlyphSlot g = face->glyph;
		FT_GlyphCache cache;
		cache.advance = g->advance;
		cache.bitmap = g->bitmap;
		cache.bitmap_top = g->bitmap_top;
		cache.bitmap_left = g->bitmap_left;
		if (g->bitmap.buffer) {
			cache.bitmap.buffer = new unsigned char[g->bitmap.width * g->bitmap.rows];
			memcpy(cache.bitmap.buffer, g->bitmap.buffer, g->bitmap.width * g->bitmap.rows);
		}
		mapping.insert(std::pair<wchar_t, FT_GlyphCache>(x, std::move(cache)));
		return true;
	}

	bool FontManager::_loadAdditionalGlyphs(const std::string& tag, wchar_t start, wchar_t end) {
		if (fontDB.find(tag) == fontDB.end()) {
			std::cerr << "** Cannot find a font by tag:" << tag << std::endl;
			return false;
		}
		if (start > end) {
			std::cerr << "** Invalid range - start is behind end" << std::endl;
			return false;
		}

		FT_Face face;
		auto glyphs = fontDB.find(tag)->second;

		if (FT_New_Face(library, glyphs->filename.c_str(), 0, &face)) {
			std::cerr << "** Cannot retrieve the font face" << std::endl;
			return false;
		}
		if (FT_Set_Pixel_Sizes(face, 0, glyphs->fontSize)) {
			std::cerr << "** Cannot set pixel size" << std::endl;
			return false;
		}

		for (wchar_t x = start; x <= end; ++x) {
			if (_loadChar(face, x, glyphs->mapping) == false) {
				return false;
			}
		}
	}

	GlyphMap* FontManager::_getGlyphMap(const std::string& tag) {
		return fontDB.find(tag)->second;
	}

}