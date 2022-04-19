#pragma once

// FreeType wrapper

#include <ft2build.h>
#include FT_FREETYPE_H

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"
#include <map>
#include <list>
#include <string>

#include "pathos/render/render_command_list.h"

#pragma comment(lib, "freetype.lib")

namespace pathos {

	class FontTextureCache;

	struct FontDesc {
		FontDesc()
			: fullFilepath()
			, pixelSize(0)
			, cacheTexture(nullptr)
		{}

		std::string fullFilepath;
		uint32 pixelSize;
		FontTextureCache* cacheTexture;
	};

	class FontManager : public Noncopyable {
		friend class FontTextureCache;

	public:
		static FontManager& get();

	public:
		bool init();
		bool term();

		bool registerFont(RenderCommandList& cmdList, const std::string& tag, const char* filepath, uint32 pixelSize);
		bool getFontDesc(const std::string& tag, FontDesc& outDesc);

		// Chance to cleanup resources related to render command execution
		void onFrameEnd();

	private:
		FontManager();
		~FontManager();

		FT_Library& getFTLibrary() { return library; }

		void registerCache(FontTextureCache* cache);
		void unregisterCache(FontTextureCache* cache);

	private:
		bool initialized = false;
		FT_Library library = nullptr;
		std::map<std::string, FontDesc> fontDescDict;
		std::list<FontTextureCache*> cacheList;
	};

}
