#include "pathos/loader/imageloader.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"

#include "badger/assertion/assertion.h"
#include <algorithm>

#define DEBUG_IMAGE_LOADER 1

namespace pathos {

	FIBITMAP* loadImage(const char* filename_) {
		std::string path = ResourceFinder::get().find(filename_);
		CHECK(path.size() != 0);

#if DEBUG_IMAGE_LOADER
		LOG(LogDebug, "load image: %s", path.c_str());
#endif

		FIBITMAP *img = NULL, *dib = NULL;
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path.c_str());
		img = FreeImage_Load(fif, path.c_str(), 0);
		if (!img) {
			LOG(LogError, "Error while loading: %s", path.c_str());
		}
		unsigned int bpp = FreeImage_GetBPP(img);
		dib = FreeImage_ConvertTo32Bits(img);
		/* 
		if (bpp == 32) {
			//
		} else if (bpp == 24) {
			dib = FreeImage_ConvertTo24Bits(img);
		} else {
			LOG(LogError, "Unexpected BPP of %d: %s", bpp, filename_);
		}
		*/
		FreeImage_Unload(img);
		return dib;
	}

	GLuint loadTexture(FIBITMAP* dib, bool generateMipmap, bool sRGB) {
		int w, h;
		unsigned char* data;
		GLuint tex_id = 0;

		data = FreeImage_GetBits(dib);
		w = FreeImage_GetWidth(dib);
		h = FreeImage_GetHeight(dib);
#if DEBUG_IMAGE_LOADER
		LOG(LogDebug, "%s: Create texture %dx%d", __FUNCTION__, w, h);
#endif

		glCreateTextures(GL_TEXTURE_2D, 1, &tex_id);

		uint32 maxLOD = static_cast<uint32>(floor(log2(std::max(w, h))) + 1);
		unsigned int bpp = FreeImage_GetBPP(dib);
		if (bpp == 32) {
			if (sRGB) {
				glTextureStorage2D(tex_id, maxLOD, GL_SRGB8_ALPHA8, w, h);
			} else {
				glTextureStorage2D(tex_id, maxLOD, GL_RGBA8, w, h);
			}
			glTextureSubImage2D(tex_id, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, data);
		} else if (bpp == 24) {
			if (sRGB) {
				glTextureStorage2D(tex_id, maxLOD, GL_SRGB8, w, h);
			} else {
				glTextureStorage2D(tex_id, maxLOD, GL_RGBA8, w, h);
			}
			glTextureSubImage2D(tex_id, 0, 0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, data);
		} else {
#ifdef _DEBUG
			LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
#endif
			glDeleteTextures(1, &tex_id);
			return 0;
		}
		if (generateMipmap) {
			glGenerateTextureMipmap(tex_id);
		}

		return tex_id;
	}

	/**
	* Generates cubemap texture from six faces.<br/>
	* image order: positiveX, negativeX, positiveY, negativeY, positiveZ, negativeZ<br/>
	* this function regards all width and height values are same as those of the first image.
	* requirements<br/>
	* <ul><li>six images must have same width and height</li>
	* <li> width and height must be same</li>
	* <li> all images have same bpp (24-bit or 32-bit)</li></ul>
	*/
	GLuint loadCubemapTexture(FIBITMAP* dib[], bool generateMipmap) {
		int w = FreeImage_GetWidth(dib[0]);
		int h = FreeImage_GetHeight(dib[0]);
		if (w != h){
			LOG(LogError, "%s: Cubemap texture must have same width and height", __FUNCTION__);
			return 0;
		}

		GLuint tex_id;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &tex_id);
		glTextureParameteri(tex_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTextureParameteri(tex_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(tex_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(tex_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		unsigned int bpp = FreeImage_GetBPP(dib[0]);
		if (bpp == 32 || bpp == 24) {
			uint32 maxLOD = generateMipmap ? static_cast<uint32>(floor(log2(std::max(w, h))) + 1) : 0;
			glTextureStorage2D(tex_id, maxLOD, GL_RGBA8, w, h);
		} else {
			LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
			return 0;
		}

		int mapping[6] = { 0, 1, 3, 2, 4, 5 };
		//int mapping[6] = { 0, 1, 2, 3, 4, 5 };
		for (int i = 0; i < 6; i++){
			unsigned char* data = FreeImage_GetBits(dib[i]);
			GLenum format = bpp == 32 ? GL_BGRA : GL_BGR;
			glTextureSubImage3D(tex_id, 0,
				0, 0, mapping[i],
				w, h, 1,
				format, GL_UNSIGNED_BYTE, data);
		}
		if (generateMipmap) {
			glGenerateTextureMipmap(tex_id);
		}

		return tex_id;
	}

}
