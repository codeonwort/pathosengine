#pragma once

#include "gl_core.h"
#include <FreeImage.h>

namespace pathos {

	void initializeImageLibrary();
	void destroyImageLibrary();

	struct HDRImageMetadata {
		float* data;
		uint32 width;
		uint32 height;
	};

	// Load bitmap images by FreeImage
	FIBITMAP* loadImage(const char* inFilename);
	GLuint createTextureFromBitmap(FIBITMAP* dib, bool generateMipmap, bool sRGB);
	GLuint createCubemapTextureFromBitmap(FIBITMAP* dib[], bool generateMipmap = true);

	// Load HDR image by stb_image
	HDRImageMetadata loadHDRImage(const char* inFilename);
	GLuint createTextureFromHDRImage(const HDRImageMetadata& metadata, bool deleteBlobData = true);
	void unloadHDRImage(const HDRImageMetadata& metadata);

}
