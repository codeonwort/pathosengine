#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/util/image_data.h"
#include <array>

namespace pathos {

	// Called at engine startup.
	void initializeImageLibrary();

	// Called at engine termination.
	void destroyImageLibrary();

}

// #wip: Cleanup image loading API
// - Unify SDR and HDR blob types.
// - Unify SDR and HDR loading API.
// - loadImage() forces 24 or 32 bpp and may cause a problem.

namespace pathos {

	/// <summary>
	/// Specify which shader language's convention the cubemap image files follow.
	/// </summary>
	enum class ECubemapImagePreference : uint8 {
		HLSL, // Image files are optimal for HLSL. Image loading API will rearrange and flip the images.
		GLSL, // Image files are optimal for GLSL. It's OK to load the images as is.
	};

	// #wip: Move image API functions into this class.
	class ImageUtils {

		/// <summary>
		/// Load image data from an image file.
		/// </summary>
		/// <param name="inFilename">Absolute path, or relative path recognized by ResourceFinder.</param>
		/// <param name="flipHorizontal">Flip the image data horizontally. Might be needed if the image does not follow GLSL convention.</param>
		/// <param name="flipVertical">Flip the image data vertically. Might be needed if the image does not follow GLSL convention.</param>
		/// <returns>A wrapper struct for the image data. Null if loading has failed.</returns>
		static ImageBlob* loadImage(const char* inFilename, bool flipHorizontal = false, bool flipVertical = false);

		/// <summary>
		/// Create a 2D texture from an image blob.
		/// NOTE: It will flush the render thread.
		/// </summary>
		/// <param name="imageBlob">Image data.</param>
		/// <param name="generateMipmaps">Auto-generate mipmaps for the texture.</param>
		/// <param name="sRGB">Image data is considered to be in sRGB color space.</param>
		/// <param name="autoDestroyBlob">Automatically deallocate `imageBlob` after it's uploaded to GPU. If false, you should free it manually.</param>
		/// <param name="debugName">Debug name of the GL texture that will be returned.</param>
		/// <returns>GL texture name.</returns>
		GLuint createTextureFromImage(
			ImageBlob* imageBlob,
			bool generateMipmaps,
			bool sRGB,
			bool autoDestroyImageBlob = true,
			const char* debugName = nullptr);

	};
}

// #wip: Old image API
namespace pathos {

	class VolumeTexture;

	/// <summary>
	/// SDR image raw data that was loaded from image files or generated procedurally.
	/// </summary>
	struct BitmapBlob {
		BitmapBlob(void* inFIBITMAP, bool inHasOpacity = false);
		BitmapBlob(uint8* inRawBytes, uint32 inWidth, uint32 inHeight, uint32 inBpp, bool inHasOpacity);
		~BitmapBlob();

		uint8* getRawBytes() const;

		void* fiHandle = nullptr; // FIBITMAP
		uint8* externalRawBytes = nullptr;
		bool bIsBGR = false; // FreeImage thing

		uint32 width = 0;
		uint32 height = 0;
		uint32 bpp = 0;
		bool hasOpacity = false;
	};

	/// <summary>
	/// HDR image raw data that was loaded from image files or generated procedurally.
	/// </summary>
	struct HDRImageBlob {
		~HDRImageBlob();

		float* rawData;
		uint32 width;
		uint32 height;
	};

	/// <summary>
	/// Load SDR image data from an image file.
	/// </summary>
	/// <param name="inFilename">Absolute path, or relative path recognized by ResourceFinder.</param>
	/// <param name="flipHorizontal">Flip the image data horizontally. Might be needed if the image does not follow GLSL convention.</param>
	/// <param name="flipVertical">Flip the image data vertically. Might be needed if the image does not follow GLSL convention.</param>
	/// <returns>A wrapper struct for the image data. Null if loading failed.</returns>
	BitmapBlob* loadImage(
		const char* inFilename,
		bool flipHorizontal = false,
		bool flipVertical = false);

	/// <summary>
	/// Load cubemap image data from 6 image files.
	/// </summary>
	/// <param name="inFilenames">Absolute paths, or relative paths recognized by ResourceFinder.</param>
	/// <param name="preference">Hint for convention of image orders and orientations.</param>
	/// <param name="outImages">Image data for each cubemap face. An element is null if the corresponding file was unable to load.</param>
	/// <returns>The number of successfully loaded files. Should be 6.</returns>
	int32 loadCubemapImages(
		const std::array<const char*,6>& inFilenames,
		ECubemapImagePreference preference,
		std::array<BitmapBlob*,6>& outImages);

	/// <summary>
	/// Load HDR image data from an image file.
	/// </summary>
	/// <param name="inFilename">Absolute paths, or relative paths recognized by ResourceFinder.</param>
	/// <returns>A wrapper struct for the image data. Null if loading failed.</returns>
	HDRImageBlob* loadHDRImage(const char* inFilename);
	
	// #wip: Don't return VolumeTexture
	/// <summary>
	/// Load image data for volume texture from a .tga file.
	/// NOTE: You should call VolumeTexture::initGLResource() manually.
	/// </summary>
	/// <param name="inFilename">Absolute paths, or relative paths recognized by ResourceFinder.</param>
	/// <param name="inDebugName">Debug name of the GL texture that will be created.</param>
	/// <returns>A wrapper for image data and GL texture.</returns>
	VolumeTexture* loadVolumeTextureFromTGA(const char* inFilename, const char* inDebugName = nullptr);

	/// <summary>
	/// Write SDR image data to an image file.
	/// </summary>
	/// <param name="width">Image width.</param>
	/// <param name="height">Image height.</param>
	/// <param name="blob">RGB data, each channel is assumed 8-bit.</param>
	/// <param name="filename">Absolute path to write an image file.</param>
	void savePNG_RGB(int32 width, int32 height, uint8* blob, const char* filename);

	/// <summary>
	/// Create a 2D texture from SDR image data.
	/// NOTE: It will flush the render thread.
	/// </summary>
	/// <param name="dib">Image raw data</param>
	/// <param name="generateMipmap">Auto-generate mipmaps for the texture.</param>
	/// <param name="sRGB">Image data is for sRGB format.</param>
	/// <param name="debugName">Debug name of the GL texture that will be created.</param>
	/// <param name="autoDestroyBlob">Deallocate the image data after it's uploaded to GPU.</param>
	/// <returns>GL texture name.</returns>
	GLuint createTextureFromBitmap(
		BitmapBlob* dib,
		bool generateMipmap,
		bool sRGB,
		const char* debugName = nullptr,
		bool autoDestroyBlob = true);

	/// <summary>
	/// Create a cubemap texture from cubemap image data array. Image order is [+X, -X, +Y, -Y, +Z, -Z].
	/// All images must have the same width, height, and bpp.
	/// NOTE: It will flush the render thread.
	/// </summary>
	/// <param name="dib">Image data for cubemap faces. The length is assumed to be 6.</param>
	/// <param name="generateMipmap">Auto-generate mipmaps for the texture.</param>
	/// <param name="debugName">Debug name of the GL texture that will be created.</param>
	/// <param name="autoDestroyBlob">Deallocate the image data after it's uploaded to GPU.</param>
	/// <returns>GL texture name.</returns>
	GLuint createCubemapTextureFromBitmap(
		BitmapBlob* dib[],
		bool generateMipmap = true,
		const char* debugName = nullptr,
		bool autoDestroyBlob = true);

	/// <summary>
	/// Create a 2D texture from HDR image data.
	/// NOTE: It will flush the render thread.
	/// </summary>
	/// <param name="blob">Image raw data</param>
	/// <param name="deleteBlobData">Deallocate the image data after it's uploaded to GPU</param>
	/// <param name="debugName">Debug name of the GL texture that will be created</param>
	/// <returns>GL texture name.</returns>
	GLuint createTextureFromHDRImage(HDRImageBlob* blob, bool deleteBlobData = true, const char* debugName = nullptr);

}
