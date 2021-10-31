#include "font_mgr.h"
#include "pathos/util/log.h"

namespace pathos {

	FontManager& FontManager::get()
	{
		static FontManager instance;
		return instance;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// non-static
	FontManager::FontManager() {}
	FontManager::~FontManager() {
		if (initialized) {
			term();
		}
	}

	bool FontManager::init() {
		if (initialized) {
			return true;
		}

		if (FT_Init_FreeType(&library)) {
			LOG(LogError, "Cannot init FreeType library");
			return false;
		}
		FT_Int major, minor, patch;
		FT_Library_Version(library, &major, &minor, &patch);
		LOG(LogInfo, "[ThirdParty] FreeType : %d.%d.%d", major, minor, patch);

		initialized = true;
		return initialized;
	}

	bool FontManager::term() {
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
			LOG(LogError, "[ThirdParty] Error while destroying FreeType library");
			return false;
		}
		return true;
	}

	bool FontManager::loadFont(const std::string& tag, const char* name, unsigned int size) {
		FT_Face face;
		std::wstring ary = L" ";
		for (wchar_t x = 33; x <= 126; ++x) ary += x;

		if (FT_New_Face(library, name, 0, &face)) {
			LOG(LogError, "Cannot retrieve font face");
			return false;
		}
		if (FT_Set_Pixel_Sizes(face, 0, size)) {
			LOG(LogError, "Cannot set pixel size");
			return false;
		}

		GlyphMap* map = new GlyphMap;
		map->filename = name;
		map->fontSize = size;
		map->maxHeight = face->size->metrics.height >> 6;

		for (auto it = ary.begin(); it != ary.end(); ++it) {
			wchar_t x = *it;
			if (loadChar(face, x, map->mapping) == false) {
				return false;
			}
		}
		fontDB[tag] = map;

		FT_Done_Face(face);

		return true;
	}

	bool FontManager::loadChar(FT_Face face, wchar_t x, std::map<wchar_t, FT_GlyphCache>& mapping) {
		if (FT_Load_Char(face, x, FT_LOAD_RENDER)) {
			LOG(LogError, "Cannot load a character: %wc", x);
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

	bool FontManager::loadAdditionalGlyphs(const std::string& tag, wchar_t start, wchar_t end) {
		if (fontDB.find(tag) == fontDB.end()) {
			LOG(LogError, "Cannot find a font by tag: %s", tag.data());
			return false;
		}
		if (start > end) {
			LOG(LogError, "Invalid range - start is behind end");
			return false;
		}

		FT_Face face;
		auto glyphs = fontDB.find(tag)->second;

		if (FT_New_Face(library, glyphs->filename.c_str(), 0, &face)) {
			LOG(LogError, "Cannot retrieve the font face");
			return false;
		}
		if (FT_Set_Pixel_Sizes(face, 0, glyphs->fontSize)) {
			LOG(LogError, "Cannot set pixel size");
			return false;
		}

		for (wchar_t x = start; x <= end; ++x) {
			if (loadChar(face, x, glyphs->mapping) == false) {
				return false;
			}
		}
		return true;
	}

	GlyphMap* FontManager::getGlyphMap(const std::string& tag) {
		return fontDB.find(tag)->second;
	}

}
