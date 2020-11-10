#pragma once

#include "gl_core.h"
#include <FreeImage.h>

#include <array>

namespace pathos {

	class VolumeTexture;

	void initializeImageLibrary();
	void destroyImageLibrary();

	struct HDRImageMetadata {
		float* data;
		uint32 width;
		uint32 height;
	};

	enum class ECubemapImagePreference : uint8 {
		HLSL, // Images are optimal for HLSL. Need to rearrange and flip the images.
		GLSL, // It's OK to load the images as is.
	};

	// #todo-image-loader: Don't expose FIBITMAP
	// Load bitmap images by FreeImage
	FIBITMAP* loadImage(const char* inFilename, bool flipHorizontal = false, bool flipVertical = false);

	// #todo-image-loader: Don't expose FIBITMAP
	// Load bitmap images for cubemap. Return value is the number of faces successfully loaded.
	int32 loadCubemapImages(const std::array<const char*,6>& inFilenames, ECubemapImagePreference preference, std::array<FIBITMAP*,6>& outImages);

	GLuint createTextureFromBitmap(FIBITMAP* dib, bool generateMipmap, bool sRGB);
	GLuint createCubemapTextureFromBitmap(FIBITMAP* dib[], bool generateMipmap = true);

	// Load HDR image by stb_image
	HDRImageMetadata loadHDRImage(const char* inFilename);
	GLuint createTextureFromHDRImage(const HDRImageMetadata& metadata, bool deleteBlobData = true);
	void unloadHDRImage(const HDRImageMetadata& metadata);

	// NOTE: You should call VolumeTexture::initGLResource() manually.
	VolumeTexture* loadVolumeTextureFromTGA(const char* inFilename, const char* inDebugName = nullptr);

}
