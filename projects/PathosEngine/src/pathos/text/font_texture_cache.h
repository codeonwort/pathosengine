#pragma once

// Rather than copying each glyph to a texture everytime when render a text,
// let's hold glyphs in a texture and only pass (u,v,w,h)

// But Unicode characters are too many that they cannot be stored in a single texture.
// Only recently used glyphs are maintained in a limited number of textures.

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>
#include <list>
#include <map>
#include <utility> // std::pair

namespace pathos {

	struct GlyphInTexture {
		wchar_t ch;
		// values in texture space
		float x, y, width, height;
		float advanceX, offsetY;
		//int age; // how long it had been not used
	};

	struct Cache {
		std::list<GlyphInTexture> used;
		std::list<GlyphInTexture> unused;
	};

	// Make an instance for each face with a size
	class FontTextureCache {

		constexpr static unsigned int TEXTURE_WIDTH = 512;
		constexpr static unsigned int TEXTURE_HEIGHT = 512;

	public:
		FontTextureCache();
		FontTextureCache(const FontTextureCache& other) = delete;
		FontTextureCache(FontTextureCache&& rhs) = delete;
		~FontTextureCache();

		bool init(FT_Library& library, const char* filename, unsigned int size);
		void term();

		inline void startGetGlyph() { glPixelStorei(GL_UNPACK_ALIGNMENT, 1); }
		const GlyphInTexture getGlyph(wchar_t x);
		inline void endGetGlyph() { glPixelStorei(GL_UNPACK_ALIGNMENT, 4); }

		inline GLuint getTexture() { return texture; }
		inline float getCellWidth() const { return static_cast<float>(maxWidth) / TEXTURE_WIDTH; }
		inline float getCellHeight() const { return static_cast<float>(maxHeight) / TEXTURE_HEIGHT; }

		inline unsigned int getTextureWidth() const { return TEXTURE_WIDTH; }
		inline unsigned int getTextureHeight() const { return TEXTURE_HEIGHT; }

	protected:
		bool contains(wchar_t x); // does it exist in one of caches?
		bool insert(wchar_t x); // returns true if successful, false otherwise
		inline bool isFull(Cache& cache) const;

	private:
		FT_Face face;

		unsigned int maxWidth, maxHeight; // max dimension among all glyphs in the font face
		unsigned int cols, rows;

		GLuint texture;
		Cache cache;

	};

}