#pragma once

// Rather than copying each glyph to a texture everytime when render a text,
// let's hold glyphs in a texture and only pass (u,v,w,h)

// But Unicode characters are too many that they cannot be stored in a single texture.
// Only recently used glyphs are maintained in a limited number of textures.

#include "pathos/render/render_command_list.h"
#include "pathos/text/font_mgr.h"
#include "gl_core.h"

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"
#include "badger/memory/mem_alloc.h"
#include <list>
#include <map>
#include <utility> // std::pair

namespace pathos {

	// A cache consists of cells of uniform size.
	struct GlyphInTexture {
		wchar_t ch;              // Character
		float x, y;              // Position in uv space
		float width, height;     // Cell size in uv space (in fact same for all GlyphInTexture instances, for now)
		float advanceX, offsetY;
		//int age;               // How long it had been not used

		float glyphWidth;        // Width in uv space
		float glyphHeight;       // Height in uv space
		int glyphPixelsX;        // Width in texels
		int glyphPixelsY;        // Height in texels
	};

	struct CacheState {
		std::list<GlyphInTexture> used;
		std::list<GlyphInTexture> unused;
	};

	// Usage strategy: Keep a global cache for general text rendering, but allow creation of private caches if needed.
	// #todo-text: Ideally wanna use just one bin-packing atlas for any font and size, but for now create a cache for each combination of font and size.
	//             Accordingly a Label is only able to express single font and size at a time, for now.
	class FontTextureCache : public Noncopyable {
		friend class FontManager;

		constexpr static uint32 TEXTURE_WIDTH = 512;
		constexpr static uint32 TEXTURE_HEIGHT = 512;

	public:
		FontTextureCache();
		~FontTextureCache();

		// #todo-text: Ideal interface for bin-packing atlas
		//bool init(uint32 textureWidth, uint32 textureHeight, uint32 numAtlases);
		//bool insert(RenderCommandList& cmdList, wchar_t x, const SomeFontDesc& fontDesc);
		//GLuint getCacheTexture() const; // could be a texture2d array

		// #todo-text: Remove filename and pixelSize parameters
		bool init(const char* filename, uint32 pixelSize);
		void term();

		void startGetGlyph(RenderCommandList& cmdList);
		const GlyphInTexture getGlyph(RenderCommandList& cmdList, wchar_t x);
		void endGetGlyph(RenderCommandList& cmdList);

		inline GLuint getTexture() { return texture; }
		inline float getCellWidth() const { return static_cast<float>(maxWidth) / TEXTURE_WIDTH; }
		inline float getCellHeight() const { return static_cast<float>(maxHeight) / TEXTURE_HEIGHT; }

		inline uint32 getTextureWidth() const { return TEXTURE_WIDTH; }
		inline uint32 getTextureHeight() const { return TEXTURE_HEIGHT; }

	protected:
		bool contains(wchar_t x); // does it exist in one of caches?
		bool insert(RenderCommandList& cmdList, wchar_t x); // returns true if successful, false otherwise
		inline bool isFull(CacheState& cache) const;

		void onFrameEnd();

	private:
		// A handle to a typographic face object. A face object models a given typeface, in a given style.
		FT_Face face = nullptr;

		// Max dimension among all glyphs in the font face
		uint32 maxWidth = 0;
		uint32 maxHeight = 0;

		// Num columns and rows of the glyph grid in the cache texture
		uint32 cols = 0;
		uint32 rows = 0;

		GLuint texture = 0;
		CacheState cacheState;
		StackAllocator glyphBufferAllocator;

	};

}
