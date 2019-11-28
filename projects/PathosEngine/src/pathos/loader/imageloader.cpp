#include "pathos/loader/imageloader.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"

#include <algorithm>
#include <assert.h>

#define DEBUG_IMAGE_LOADER 1

namespace pathos {

	FIBITMAP* loadImage(const char* filename_) {
		std::string path = ResourceFinder::get().find(filename_);
		assert(path.size() != 0);

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

		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_2D, tex_id);

		unsigned int numMipmaps = static_cast<unsigned int>(floor(log2(std::max(w, h))) + 1);
		unsigned int bpp = FreeImage_GetBPP(dib);
		if (bpp == 32) {
			if (sRGB) {
				glTexStorage2D(GL_TEXTURE_2D, numMipmaps, GL_SRGB8_ALPHA8, w, h);
			} else {
				glTexStorage2D(GL_TEXTURE_2D, numMipmaps, GL_RGBA8, w, h);
			}
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, data);
		} else if (bpp == 24) {
			if (sRGB) {
				glTexStorage2D(GL_TEXTURE_2D, numMipmaps, GL_SRGB8, w, h);
			} else {
				glTexStorage2D(GL_TEXTURE_2D, numMipmaps, GL_RGBA8, w, h);
			}
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, data);
		} else {
#ifdef _DEBUG
			LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
#endif
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &tex_id);
			return 0;
		}
		if (generateMipmap) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		return tex_id;
	}

	/**
	* generate cubemap texture from six faces.<br/>
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
		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		unsigned int bpp = FreeImage_GetBPP(dib[0]);
		if (bpp == 32 || bpp == 24) {
			glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA32F, w, h);
		} else {
			LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
			return 0;
		}

		int mapping[6] = { 0, 1, 3, 2, 4, 5 };
		//int mapping[6] = { 0, 1, 2, 3, 4, 5 };
		for (int i = 0; i < 6; i++){
			unsigned char* data = FreeImage_GetBits(dib[i]);
			if (bpp == 32) {
				glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + mapping[i], 0, 0,0,w,h, GL_BGRA, GL_UNSIGNED_BYTE, data);
			}else if (bpp == 24) {
				glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + mapping[i], 0, 0,0,w,h, GL_BGR, GL_UNSIGNED_BYTE, data);
			}
		}
		if (generateMipmap) {
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		return tex_id;
	}

}
