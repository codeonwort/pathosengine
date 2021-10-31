#pragma once

// FreeType wrapper

#include <ft2build.h>
#include FT_FREETYPE_H

#include "badger/types/noncopyable.h"
#include <map>
#include <list>
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

	// #todo-text: First thought was this is redundant with FontTextureCache, but this is needed as a general FT wrapper.
	class FontManager : public Noncopyable {
		friend class FontTextureCache;

	public:
		static FontManager& get();

	public:
		bool init();
		bool term();
		bool loadFont(const std::string& tag, const char* name, unsigned int size);
		bool loadAdditionalGlyphs(const std::string& tag, wchar_t start, wchar_t end);
		GlyphMap* getGlyphMap(const std::string& tag);

		// Chance to cleanup resources related to render command execution
		void onFrameEnd();

	private:
		FontManager();
		~FontManager();

		FT_Library& getFTLibrary() { return library; }

		bool loadChar(FT_Face face, wchar_t x, std::map<wchar_t, FT_GlyphCache>& mapping);

		void registerCache(FontTextureCache* cache);
		void unregisterCache(FontTextureCache* cache);

	private:
		bool initialized = false;
		FT_Library library = nullptr;
		FontDB fontDB;
		std::list<FontTextureCache*> cacheList;
	};

}
