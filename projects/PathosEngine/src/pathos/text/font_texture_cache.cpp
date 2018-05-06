
#include "font_texture_cache.h"

namespace pathos {

	FontTextureCache::FontTextureCache() {
		//
	}

	FontTextureCache::~FontTextureCache() {
		term();
	}

	bool FontTextureCache::init(FT_Library& library, const char* filename, unsigned int size) {
		// FreeType
		if (FT_New_Face(library, filename, 0, &face) != 0) {
			return false;
		}
		if (FT_Set_Pixel_Sizes(face, 0, size) != 0) {
			return false;
		}

		// cache
		maxWidth = face->size->metrics.max_advance >> 6;
		maxHeight = face->size->metrics.height >> 6;
		cols = TEXTURE_WIDTH / maxWidth;
		rows = TEXTURE_HEIGHT / maxHeight;
		float ratioX = static_cast<float>(maxWidth) / TEXTURE_WIDTH;
		float ratioY = static_cast<float>(maxHeight) / TEXTURE_HEIGHT;
		
		for (auto y = 0u; y < rows; ++y) {
			for (auto x = 0u; x < cols; ++x) {
				GlyphInTexture g;
				g.ch = 0;
				g.x = x * ratioX;
				g.y = y * ratioY;
				g.width = ratioX;
				g.height = ratioY;
				cache.unused.emplace_back(g);
			}
		}

		// OpenGL
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, TEXTURE_WIDTH, TEXTURE_HEIGHT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return true;
	}

	void FontTextureCache::term() {
		FT_Done_Face(face);
		glDeleteTextures(1, &texture);
		cache.used.clear();
		cache.unused.clear();
	}

	const GlyphInTexture FontTextureCache::getGlyph(wchar_t x) {
		// TODO: currently use only first cache
		insert(x);
		const auto& v = cache.used;
		for(auto& g : v) if (g.ch == x) return g;
		return GlyphInTexture{};
	}

	bool FontTextureCache::contains(wchar_t x) {
		// TODO: currently use only first cache
		const auto& v = cache.used;
		for (auto& g : v) if (g.ch == x) return true;
		return false;
	}

	bool FontTextureCache::insert(wchar_t x) {
		if (contains(x)) return true;

		GlyphInTexture g;

		// TODO: currently only use first cache
		Cache& c = cache;
		if (isFull(c)) {
			// TODO: profiling for cache miss
			g = c.used.front();
			c.used.pop_front();
		} else {
			g = c.unused.back();
			c.unused.pop_back();
		}

		if (FT_Load_Char(face, x, FT_LOAD_RENDER) != 0) {
			return false;
		}

		g.advanceX = static_cast<float>(face->glyph->advance.x >> 6) / TEXTURE_WIDTH;
		g.offsetY = static_cast<float>(face->glyph->bitmap_top) / TEXTURE_HEIGHT;
		g.ch = x;
		c.used.push_back(g);

		FT_Bitmap& bmp = face->glyph->bitmap;
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, (GLint)(g.x * TEXTURE_WIDTH), (GLint)(g.y * TEXTURE_HEIGHT), bmp.width, bmp.rows, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);

		return true;
	}

	bool FontTextureCache::isFull(Cache& cache_) const { return cache_.unused.size() == 0; }

}