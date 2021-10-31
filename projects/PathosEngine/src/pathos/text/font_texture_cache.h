#pragma once

// Rather than copying each glyph to a texture everytime when render a text,
// let's hold glyphs in a texture and only pass (u,v,w,h)

// But Unicode characters are too many that they cannot be stored in a single texture.
// Only recently used glyphs are maintained in a limited number of textures.

#include <ft2build.h>
#include FT_FREETYPE_H

#include "pathos/render/render_command_list.h"
#include "gl_core.h"

#include "badger/types/int_types.h"
#include <list>
#include <map>
#include <utility> // std::pair

namespace pathos {

	struct GlyphInTexture {
		wchar_t ch;
		float x, y; // position in texture space
		float width, height; // cell size in texture space
		float advanceX, offsetY;
		//int age; // how long it had been not used

		float glyphWidth;
		float glyphHeight;
		int glyphPixelsX;
		int glyphPixelsY;
	};

	struct Cache {
		std::list<GlyphInTexture> used;
		std::list<GlyphInTexture> unused;
	};

	// Usage strategy: Keep a global cache for general text rendering, but allow creation of private caches if needed.
	class FontTextureCache {

		constexpr static uint32 TEXTURE_WIDTH = 512;
		constexpr static uint32 TEXTURE_HEIGHT = 512;

	public:
		FontTextureCache();
		FontTextureCache(const FontTextureCache& other) = delete;
		FontTextureCache(FontTextureCache&& rhs) = delete;
		~FontTextureCache();

		// #todo-text: Remove pixelSize parameter
		bool init(FT_Library& library, const char* filename, uint32 pixelSize);
		void term();

		inline void startGetGlyph(RenderCommandList& cmdList) { cmdList.pixelStorei(GL_UNPACK_ALIGNMENT, 1); }
		const GlyphInTexture getGlyph(RenderCommandList& cmdList, wchar_t x);
		inline void endGetGlyph(RenderCommandList& cmdList) { cmdList.pixelStorei(GL_UNPACK_ALIGNMENT, 4); }

		inline GLuint getTexture() { return texture; }
		inline float getCellWidth() const { return static_cast<float>(maxWidth) / TEXTURE_WIDTH; }
		inline float getCellHeight() const { return static_cast<float>(maxHeight) / TEXTURE_HEIGHT; }

		inline unsigned int getTextureWidth() const { return TEXTURE_WIDTH; }
		inline unsigned int getTextureHeight() const { return TEXTURE_HEIGHT; }

	protected:
		bool contains(wchar_t x); // does it exist in one of caches?
		bool insert(RenderCommandList& cmdList, wchar_t x); // returns true if successful, false otherwise
		inline bool isFull(Cache& cache) const;

	private:
		FT_Face face;

		unsigned int maxWidth, maxHeight; // max dimension among all glyphs in the font face
		unsigned int cols, rows;

		GLuint texture;
		Cache cache;

	};

}