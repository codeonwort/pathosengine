#include "font_texture_cache.h"
#include "pathos/render/render_device.h"
#include <vector>

#define DRAW_GLYPH_BORDER 0

namespace pathos {

	FontTextureCache::FontTextureCache() {
		//
	}

	FontTextureCache::~FontTextureCache() {
		term();
	}

	bool FontTextureCache::init(FT_Library& library, const char* filename, uint32 pixelSize) {
		// FreeType
		if (FT_New_Face(library, filename, 0, &face) != 0) {
			return false;
		}
		if (FT_Set_Pixel_Sizes(face, 0, pixelSize) != 0) {
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
		gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &texture);

		GLuint textureName = texture;
		ENQUEUE_RENDER_COMMAND([textureName](RenderCommandList& cmdList) {
			cmdList.textureStorage2D(textureName, 1, GL_R8, TEXTURE_WIDTH, TEXTURE_HEIGHT);
			cmdList.textureParameteri(textureName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			cmdList.textureParameteri(textureName, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(textureName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(textureName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		});

		return true;
	}

	void FontTextureCache::term() {
		FT_Done_Face(face);
		gRenderDevice->deleteTextures(1, &texture);
		cache.used.clear();
		cache.unused.clear();
	}

	const GlyphInTexture FontTextureCache::getGlyph(RenderCommandList& cmdList, wchar_t x) {
		// #todo-text: currently use only first cache
		insert(cmdList, x);
		const auto& v = cache.used;
		for(auto& g : v) if (g.ch == x) return g;
		return GlyphInTexture{};
	}

	bool FontTextureCache::contains(wchar_t x) {
		// #todo-text: currently use only first cache
		const auto& v = cache.used;
		for (auto& g : v) if (g.ch == x) return true;
		return false;
	}

	bool FontTextureCache::insert(RenderCommandList& cmdList, wchar_t x) {
		if (contains(x)) {
			return true;
		}

		GlyphInTexture g;

		// #todo-text: currently only use first cache
		Cache& c = cache;
		if (isFull(c)) {
			// #todo-text: profiling for cache miss
			g = c.used.front();
			c.used.pop_front();
		} else {
			g = c.unused.back();
			c.unused.pop_back();
		}

		// #todo-text: It overwrites previous face data. Need to batch all insert() in one frame.
		if (FT_Load_Char(face, x, FT_LOAD_RENDER) != 0) {
			return false;
		}

		g.advanceX = static_cast<float>(face->glyph->advance.x >> 6) / TEXTURE_WIDTH;
		g.offsetY = static_cast<float>(face->glyph->bitmap_top) / TEXTURE_HEIGHT;
		g.ch = x;

		FT_Bitmap& bmp = face->glyph->bitmap;

		g.glyphPixelsX = bmp.width;
		g.glyphPixelsY = bmp.rows;
		g.glyphWidth = static_cast<float>(bmp.width) / TEXTURE_WIDTH;
		g.glyphHeight = static_cast<float>(bmp.rows) / TEXTURE_HEIGHT;

		c.used.push_back(g);

		/* flip bmp
		std::vector<unsigned char> flipped(bmp.width * bmp.rows, 0);
		unsigned char* p = nullptr;
		if(flipped.size() > 0) {
			p = &flipped[0];
			for (int i = bmp.rows - 1; i >= 0; --i) {
				memcpy(p, bmp.buffer + i * bmp.width, bmp.width);
				p += bmp.width;
			}
		}
		p = flipped.size() == 0 ? nullptr : &flipped[0];
		*/
		
		unsigned char* glyphBuffer = bmp.buffer;

#if DRAW_GLYPH_BORDER
		const size_t bufferSize = bmp.width * bmp.rows;
		glyphBuffer = new unsigned char[bufferSize];
		memcpy_s(glyphBuffer, bufferSize, bmp.buffer, bufferSize);
		for (auto y = 0u; y < bmp.rows; ++y) {
			glyphBuffer[y * bmp.width] = 0xff;
			glyphBuffer[y * bmp.width + bmp.width - 1] = 0xff;
		}
		for (auto x = 0u; x < bmp.width; ++x) {
			glyphBuffer[x] = 0xff;
			glyphBuffer[(bmp.rows - 1) * bmp.width + x] = 0xff;
		}
#endif

		cmdList.textureSubImage2D(
			texture,                          // texture
			0,                                // level
			(GLint)(g.x * TEXTURE_WIDTH),     // xoffset
			(GLint)(g.y * TEXTURE_HEIGHT),    // yoffset
			bmp.width, bmp.rows,              // width, heigth
			GL_RED, GL_UNSIGNED_BYTE,         // format, type
			glyphBuffer);                     // pixels
		
		// #todo-text: Hold glyphBuffer until all commands are finalized. Temp flush to make it work right now...
		cmdList.flushAllCommands();

#if DRAW_GLYPH_BORDER
		delete[] glyphBuffer;
#endif

		return true;
	}

	bool FontTextureCache::isFull(Cache& cache_) const { return cache_.unused.size() == 0; }

}
