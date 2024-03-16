#include "font_texture_cache.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/texture.h"
#include "pathos/util/log.h"

#define DRAW_GLYPH_BORDER 0 // Debug
#define FLIP_BITMAP_DATA  0 // No need for GLSL

// Temporarily hold data for textureSubImage2D calls. Cleared on frame end.
#define GLYPH_BUFFER_MAX_SIZE (2 * 1024 * 1024) // 2 MB

namespace pathos {

	static uint32 g_fontTextureCacheNumber = 0;

	FontTextureCache::FontTextureCache()
		: glyphBufferAllocator(GLYPH_BUFFER_MAX_SIZE)
	{
	}

	FontTextureCache::~FontTextureCache() {
		term();
	}

	bool FontTextureCache::init(RenderCommandList& cmdList, const char* filename, uint32 pixelSize) {
		// FreeType
		FT_Library& library = FontManager::get().getFTLibrary();
		if (FT_New_Face(library, filename, 0, &face) != 0) {
			return false;
		}
		if (FT_Set_Pixel_Sizes(face, 0, pixelSize) != 0) {
			return false;
		}

		// Prepare glyph cells
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
				cacheState.unused.emplace_back(g);
			}
		}

		// Prepare the cache texture
		char textureObjectLabel[256];
		sprintf_s(textureObjectLabel, "FontTextureCache%d", g_fontTextureCacheNumber++);
		
		TextureCreateParams createParams;
		createParams.width = TEXTURE_WIDTH;
		createParams.height = TEXTURE_HEIGHT;
		createParams.depth = 1;
		createParams.mipLevels = 1;
		createParams.glDimension = GL_TEXTURE_2D;
		createParams.glStorageFormat = GL_R8;
		createParams.debugName = textureObjectLabel;
		
		texture = new Texture(createParams);
		texture->createGPUResource();

		cmdList.textureParameteri(texture->internal_getGLName(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureParameteri(texture->internal_getGLName(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(texture->internal_getGLName(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(texture->internal_getGLName(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		FontManager::get().registerCache(this);

		return true;
	}

	void FontTextureCache::term() {
		FT_Done_Face(face);
		texture->releaseGPUResource();
		FontManager::get().unregisterCache(this);
		cacheState.used.clear();
		cacheState.unused.clear();
	}

	void FontTextureCache::startGetGlyph(RenderCommandList& cmdList)
	{
		cmdList.pixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	const GlyphInTexture FontTextureCache::getGlyph(RenderCommandList& cmdList, wchar_t x) {
		insert(cmdList, x);
		const auto& v = cacheState.used;
		for (auto& g : v) {
			if (g.ch == x) {
				return g;
			}
		}
		// #todo-text: Returns a placeholder character (like '?') for page fault
		return GlyphInTexture{};
	}

	void FontTextureCache::endGetGlyph(RenderCommandList& cmdList)
	{
		cmdList.pixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

	bool FontTextureCache::contains(wchar_t x) {
		const auto& v = cacheState.used;
		for (auto& g : v) {
			if (g.ch == x) {
				return true;
			}
		}
		return false;
	}

	bool FontTextureCache::insert(RenderCommandList& cmdList, wchar_t x) {
		if (contains(x)) {
			return true;
		}

		GlyphInTexture g;

		// #todo-text: This strategy works only for uniform cells
		if (isFull(cacheState)) {
			g = cacheState.used.front();
			cacheState.used.pop_front();
		} else {
			g = cacheState.unused.back();
			cacheState.unused.pop_back();
		}

		// Overwrites previous glyph slot data.
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

		cacheState.used.push_back(g);
		
		const uint32 glyphBufferSize = (uint32)(bmp.width * bmp.rows);
		uint8* glyphBuffer = nullptr;
		// e.g., white spaces have only advanceX, no actual data
		if (glyphBufferSize != 0) {
			glyphBuffer = reinterpret_cast<uint8*>(glyphBufferAllocator.alloc(glyphBufferSize));
			CHECKF(glyphBuffer != nullptr, "Glyph buffer is full. Please increase GLYPH_BUFFER_MAX_SIZE");
			memcpy_s(glyphBuffer, glyphBufferSize, bmp.buffer, glyphBufferSize);
		}

#if FLIP_BITMAP_DATA
		if (glyphBufferSize != 0 && glyphBuffer != nullptr) {
			uint8* flipped = reinterpret_cast<uint8*>(glyphBufferAllocator.alloc(glyphBufferSize));
			for (int32 i = bmp.rows - 1; i >= 0; --i) {
				memcpy(flipped + (bmp.rows - i - 1) * bmp.width, bmp.buffer + i * bmp.width, bmp.width);
			}
			memcpy_s(glyphBuffer, glyphBufferSize, flipped, glyphBufferSize);
		}
#endif

#if DRAW_GLYPH_BORDER
		if (glyphBuffer != nullptr) {
			for (auto y = 0u; y < bmp.rows; ++y) {
				glyphBuffer[y * bmp.width] = 0xff;
				glyphBuffer[y * bmp.width + bmp.width - 1] = 0xff;
			}
			for (auto x = 0u; x < bmp.width; ++x) {
				glyphBuffer[x] = 0xff;
				glyphBuffer[(bmp.rows - 1) * bmp.width + x] = 0xff;
			}
		}
#endif

		if (glyphBufferSize != 0) {
			cmdList.textureSubImage2D(
				texture->internal_getGLName(), // texture
				0,                             // level
				(GLint)(g.x * TEXTURE_WIDTH),  // xoffset
				(GLint)(g.y * TEXTURE_HEIGHT), // yoffset
				bmp.width, bmp.rows,           // width, heigth
				GL_RED, GL_UNSIGNED_BYTE,      // format, type
				glyphBuffer);                  // pixels
		}

		return true;
	}

	bool FontTextureCache::isFull(CacheState& cache_) const {
		return cache_.unused.size() == 0;
	}

	void FontTextureCache::onFrameEnd() {
		glyphBufferAllocator.clear();
	}

}
