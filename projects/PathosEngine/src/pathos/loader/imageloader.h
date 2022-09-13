#pragma once

#include <array>

// #todo-image-loader: Wanna get rid of GLuint from this header.
typedef unsigned int GLuint;

namespace pathos {

	class VolumeTexture;

	void initializeImageLibrary();
	void destroyImageLibrary();

	struct HDRImageMetadata {
		float* data;
		uint32 width;
		uint32 height;
	};

	// - Wrapper for FreeImage's FIBITMAP.
	// - If its data is uploaded to GPU and not accessed from CPU anymore, it should be freed.
	//   It can be achieved by trasnfering memory ownership of a BitmapBlob to cmdList.
	struct BitmapBlob {
		BitmapBlob(void* inFIBITMAP);
		~BitmapBlob();

		uint8* getRawBytes() const;

		void* fiHandle = nullptr; // FIBITMAP
		uint32 width = 0;
		uint32 height = 0;
		uint32 bpp = 0;
	};

	enum class ECubemapImagePreference : uint8 {
		HLSL, // Images are optimal for HLSL. Need to rearrange and flip the images.
		GLSL, // It's OK to load the images as is.
	};

	// #todo-image-loader: This forces 24 or 32 bpp and may causes a problem.
	// Load bitmap images by FreeImage
	BitmapBlob* loadImage(
		const char* inFilename,
		bool flipHorizontal = false,
		bool flipVertical = false);

	// Load bitmap images for cubemap. Return value is the number of faces successfully loaded.
	int32 loadCubemapImages(
		const std::array<const char*,6>& inFilenames,
		ECubemapImagePreference preference,
		std::array<BitmapBlob*,6>& outImages);

	GLuint createTextureFromBitmap(
		BitmapBlob* dib,
		bool generateMipmap,
		bool sRGB,
		const char* debugName = nullptr);

	GLuint createCubemapTextureFromBitmap(
		BitmapBlob* dib[], // Assumes length of 6
		bool generateMipmap = true,
		const char* debugName = nullptr);

	// Load HDR image by stb_image
	HDRImageMetadata loadHDRImage(const char* inFilename);
	GLuint createTextureFromHDRImage(const HDRImageMetadata& metadata, bool deleteBlobData = true, const char* debugName = nullptr);
	void unloadHDRImage(const HDRImageMetadata& metadata);

	// NOTE: You should call VolumeTexture::initGLResource() manually.
	VolumeTexture* loadVolumeTextureFromTGA(const char* inFilename, const char* inDebugName = nullptr);

}
