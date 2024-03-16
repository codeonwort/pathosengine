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

namespace pathos {

	class Texture;

	/// <summary>
	/// Specify which shader language's convention the cubemap image files follow.
	/// </summary>
	enum class ECubemapImagePreference : uint8 {
		HLSL, // Image files are optimal for HLSL. Image loading API will rearrange and flip the images.
		GLSL, // Image files are optimal for GLSL. It's OK to load the images as is.
	};

	// #wip: Move image API functions into this class.
	class ImageUtils {

	public:
		/// <summary>
		/// Load image data from an image file.
		/// </summary>
		/// <param name="inFilename">Absolute path, or relative path recognized by ResourceFinder.</param>
		/// <param name="flipHorizontal">Flip the image data horizontally. Might be needed if the image does not follow GLSL convention.</param>
		/// <param name="flipVertical">Flip the image data vertically. Might be needed if the image does not follow GLSL convention.</param>
		/// <returns>A wrapper struct for the image data. Null if loading has failed.</returns>
		static ImageBlob* loadImage(const char* inFilename, bool flipHorizontal = false, bool flipVertical = false);

		/// <summary>
		/// Load cubemap image data from 6 image files.
		/// </summary>
		/// <param name="inFilenames">Absolute paths or relative paths recognized by ResourceFinder.</param>
		/// <param name="preference">Hint for convention of image orders and orientations.</param>
		/// <returns>Image data array for cubemap faces. An element is null if the corresponding face has failed to load.</returns>
		static std::vector<ImageBlob*> loadCubemapImages(const std::array<const char*,6>& inFilenames, ECubemapImagePreference preference);

		/// <summary>
		/// Create a 2D texture from an image blob.
		/// </summary>
		/// <param name="imageBlob">Image data.</param>
		/// <param name="mipLevels">The number of mip levels.</param>
		/// <param name="sRGB">Image data is considered to be in sRGB color space.</param>
		/// <param name="autoDestroyBlob">Automatically deallocate `imageBlob` after it's uploaded to GPU. If false, you should free it manually.</param>
		/// <param name="debugName">Debug name of the GL texture that will be created.</param>
		/// <returns>Texture object.</returns>
		static Texture* createTexture2DFromImage(
			ImageBlob* imageBlob,
			uint32 mipLevels,
			bool sRGB,
			bool autoDestroyImageBlob = true,
			const char* debugName = nullptr);

		/// <summary>
		/// Create a cubemap texture from cubemap image data array. Image order is [+X, -X, +Y, -Y, +Z, -Z].
		/// All images must have the same width, height, and bpp.
		/// NOTE: It will flush the render thread.
		/// </summary>
		/// <param name="imageBlobs">Image data for cubemap faces. The length is assumed to be 6.</param>
		/// <param name="mipLevels">The number of mip levels.</param>
		/// <param name="autoDestroyBlob">Deallocate the image data after it's uploaded to GPU.</param>
		/// <param name="debugName">Debug name of the GL texture that will be created.</param>
		/// <returns>Texture object.</returns>
		static Texture* createTextureCubeFromImages(
			std::vector<ImageBlob*> imageBlobs,
			uint32 mipLevels,
			bool autoDestroyImageBlob = true,
			const char* debugName = nullptr);

		/// <summary>
		/// Create a 3D texture from an image blob.
		/// </summary>
		/// <param name="imageBlob">Image data.</param>
		/// <param name="textureSize">3D size of the texture that will be created.</param>
		/// <param name="mipLevels">The number of mip levels.</param>
		/// <param name="sRGB">Image data is considered to be in sRGB color space.</param>
		/// <param name="autoDestroyBlob">Automatically deallocate `imageBlob` after it's uploaded to GPU. If false, you should free it manually.</param>
		/// <param name="debugName">Debug name of the GL texture that will be created.</param>
		/// <returns>Texture object.</returns>
		static Texture* createTexture3DFromImage(
			ImageBlob* imageBlob,
			const vector3ui& textureSize,
			uint32 mipLevels,
			bool sRGB,
			bool autoDestroyImageBlob = true,
			const char* debugName = nullptr);

		/// <summary>
		/// Write 8-bit RGB image to an image file.
		/// </summary>
		/// <param name="width">Image width.</param>
		/// <param name="height">Image height.</param>
		/// <param name="blob">RGB data, each channel is assumed 8-bit.</param>
		/// <param name="filepath">Absolute path to write an image file.</param>
		static void saveRGB8ImageAsPNG(int32 width, int32 height, uint8* blob, const char* filepath);
	};
}
