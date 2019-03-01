#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <string>

#pragma comment(lib, "freetype.lib")

namespace pathos {

	struct FT_GlyphCache {
		FT_Vector advance;
		FT_Bitmap bitmap;
		FT_Int bitmap_top;
		FT_Int bitmap_left;
	};

	struct GlyphMap {
		std::map<wchar_t, FT_GlyphCache> mapping;
		std::string filename;
		unsigned int fontSize;
		unsigned int maxHeight;

		const FT_GlyphCache& getGlyphCache(wchar_t x) {
			auto it = mapping.find(x);
			if (it == mapping.end()) return mapping.find(L'?')->second;
			return it->second;
		}
	};

	using FontDB = std::map<std::string, GlyphMap*>;

	// singleton
	class FontManager {

	// static
	public:
		static bool init() { return getInstance()->_init(); }
		static bool term() { return getInstance()->_term(); }
		static bool available() { return getInstance()->initialized; }
		static bool loadFont(const std::string& tag, const char* name, unsigned int size) { return getInstance()->_loadFont(tag, name, size); }
		static bool loadAdditionalGlyphs(const std::string& tag, wchar_t start, wchar_t end) { return getInstance()->_loadAdditionalGlyphs(tag, start, end); }
		static GlyphMap* getGlyphMap(const std::string& tag) { return getInstance()->_getGlyphMap(tag); }
		static FT_Library& getFTLibrary() { return getInstance()->library; }
	private:
		static FontManager* instance;
		static FontManager* getInstance();

	// non-static
	private:
		FontManager();
		FontManager(const FontManager& other) = delete;
		FontManager(FontManager&& rhs) = delete;
		~FontManager();

		bool initialized = false;
		FT_Library library;
		FontDB fontDB;

		bool _init();
		bool _term();
		bool _loadFont(const std::string& tag, const char* name, unsigned int size);
		bool _loadChar(FT_Face face, wchar_t x, std::map<wchar_t, FT_GlyphCache>& mapping);
		bool _loadAdditionalGlyphs(const std::string& tag, wchar_t start, wchar_t end);
		GlyphMap* _getGlyphMap(const std::string& tag);
	};

}