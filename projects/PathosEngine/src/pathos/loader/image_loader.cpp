#include "pathos/loader/image_loader.h"

#include "pathos/rhi/texture.h"
#include "pathos/rhi/render_device.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

#include "stb_image.h"
#include <FreeImage.h>

#pragma comment(lib, "FreeImage.lib")

namespace pathos {

	void initializeImageLibrary() {
		FreeImage_Initialise();
		LOG(LogInfo, "[ThirdParty] FreeImage %d.%d", FREEIMAGE_MAJOR_VERSION, FREEIMAGE_MINOR_VERSION);

		stbi_set_flip_vertically_on_load(true);
	}

	void destroyImageLibrary() {
		FreeImage_DeInitialise();
		LOG(LogInfo, "[ThirdParty] Destroy image libraries");
	}

	void FreeImage_GetGLFormats(
		FREE_IMAGE_FORMAT freeImageFormat, uint32 bpp,
		GLenum& outStorageFormat, GLenum& outPixelFormat, GLenum& outDataType)
	{
		switch (freeImageFormat) {
			case FIF_BMP:
			case FIF_JPEG:
			case FIF_PNG:
				{
					switch (bpp) {
						case 8:  outStorageFormat = GL_R8;      outPixelFormat = GL_RED;  outDataType = GL_UNSIGNED_BYTE;  break;
						case 24: outStorageFormat = GL_RGB8;    outPixelFormat = GL_BGR;  outDataType = GL_UNSIGNED_BYTE;  break;
						case 32: outStorageFormat = GL_RGBA8;   outPixelFormat = GL_BGRA; outDataType = GL_UNSIGNED_BYTE;  break;
						case 48: outStorageFormat = GL_RGB16;   outPixelFormat = GL_BGR;  outDataType = GL_UNSIGNED_SHORT; break;
						case 64: outStorageFormat = GL_RGBA16;  outPixelFormat = GL_BGRA; outDataType = GL_UNSIGNED_SHORT; break;
					}
				}
				break;
			case FIF_TARGA:
				{
					switch (bpp) {
						case 8:  outStorageFormat = GL_R8;    outPixelFormat = GL_RED;  break;
						case 16: outStorageFormat = GL_RG8;   outPixelFormat = GL_RG;   break;
						case 24: outStorageFormat = GL_RGB8;  outPixelFormat = GL_RGB;  break;
						case 32: outStorageFormat = GL_RGBA8; outPixelFormat = GL_RGBA; break;
					}
					outDataType = GL_UNSIGNED_BYTE;
				}
				break;
			case FIF_HDR:
			case FIF_EXR:
				{
					// #wip: Is 64 bpp rg32f or rgba16f? Can I discern them?
					switch (bpp) {
						case 128: outStorageFormat = GL_RGBA32F; outPixelFormat = GL_RGBA; break;
						case 96:  outStorageFormat = GL_RGB32F;  outPixelFormat = GL_RGB;  break;
					}
					outDataType = GL_FLOAT;
				}
				break;
			case FIF_UNKNOWN:
			case FIF_ICO:
			case FIF_JNG:
			case FIF_KOALA:
			case FIF_LBM: // = FIF_IFF
			case FIF_MNG:
			case FIF_PBM:
			case FIF_PBMRAW:
			case FIF_PCD:
			case FIF_PCX:
			case FIF_PGM:
			case FIF_PGMRAW:
			case FIF_PPM:
			case FIF_PPMRAW:
			case FIF_RAS:
			case FIF_TIFF:
			case FIF_WBMP:
			case FIF_PSD:
			case FIF_CUT:
			case FIF_XBM:
			case FIF_XPM:
			case FIF_DDS:
			case FIF_GIF:
			case FIF_FAXG3:
			case FIF_SGI:
			case FIF_J2K:
			case FIF_JP2:
			case FIF_PFM:
			case FIF_PICT:
			case FIF_RAW:
			case FIF_WEBP:
			case FIF_JXR:
			default:
				CHECK_NO_ENTRY();
				break;
		} // switch
		if (outStorageFormat == GL_NONE || outPixelFormat == GL_NONE || outDataType == GL_NONE) {
			LOG(LogError, "[FreeImage_GetGLPixelFormat] Can't derive GL formats from FIF=%d, bpp=%u", freeImageFormat, bpp);
		}
	}

	GLenum convertStorageFormatToSRGB(GLenum storageFormat) {
		if (storageFormat == GL_RGBA8) storageFormat = GL_SRGB8_ALPHA8;
		else if (storageFormat == GL_RGB8) storageFormat = GL_SRGB8;
		return storageFormat;
	}
}

namespace pathos {

	ImageBlob* ImageUtils::loadImage(const char* inFilename, bool flipHorizontal /*= false*/, bool flipVertical /*= false*/) {
		std::string path = ResourceFinder::get().find(inFilename);
		if (path.size() == 0) {
			LOG(LogError, "[ImageUtils::loadImage] Can't find file: %s", inFilename);
			return nullptr;
		}

		FREE_IMAGE_FORMAT freeImageFormat = FreeImage_GetFIFFromFilename(path.c_str());
		FIBITMAP* dib = FreeImage_Load(freeImageFormat, path.c_str(), 0);

		if (!dib) {
			LOG(LogError, "[ImageUtils::loadImage] Error while loading: %s", path.c_str());
			return nullptr;
		}

		if (flipHorizontal) FreeImage_FlipHorizontal(dib);
		if (flipVertical) FreeImage_FlipVertical(dib);

		unsigned int bpp = FreeImage_GetBPP(dib);
		// #wip: Weather map for volumetric clouds is broken if the file is 64-bit PNG?
		{
			FIBITMAP* oldDib = nullptr;
			if (freeImageFormat == FIF_PNG && bpp == 64) {
				oldDib = dib;
				dib = FreeImage_ConvertTo32Bits(dib);
				bpp = 32;
			} else if (freeImageFormat == FIF_PNG && bpp == 48) {
				oldDib = dib;
				dib = FreeImage_ConvertTo24Bits(dib);
				bpp = 24;
			}
			if (oldDib != nullptr) {
				FreeImage_Unload(oldDib);
			}
		}
		const BYTE* rawData = FreeImage_GetBits(dib);
		const unsigned int width = FreeImage_GetWidth(dib);
		const unsigned int height = FreeImage_GetHeight(dib);

		GLenum glStorageFormat, glPixelFormat, glDataType;
		FreeImage_GetGLFormats(freeImageFormat, bpp, glStorageFormat, glPixelFormat, glDataType);

		ImageBlob* blob = new ImageBlob;
		blob->copyRawBytes(rawData, width, height, bpp);
		blob->glStorageFormat = glStorageFormat;
		blob->glPixelFormat = glPixelFormat;
		blob->glDataType = glDataType;

		FreeImage_Unload(dib);

		return blob;
	}

	std::vector<ImageBlob*> ImageUtils::loadCubemapImages(const std::array<const char*, 6>& inFilenames, ECubemapImagePreference preference) {
		std::vector<ImageBlob*> images(6, nullptr);
		const int32 glslOrder[6] = { 0, 1, 2, 3, 5, 4 };
		for (int32 i = 0; i < 6; ++i) {
			bool flipH = preference == ECubemapImagePreference::HLSL && (i != 2 && i != 3);
			bool flipV = preference == ECubemapImagePreference::HLSL && (i != 2 && i != 3);
			int32 j = preference == ECubemapImagePreference::HLSL ? glslOrder[i] : i;
			images[i] = ImageUtils::loadImage(inFilenames[j], flipH, flipV);
		}
		return images;
	}

	Texture* ImageUtils::createTexture2DFromImage(
		ImageBlob* imageBlob,
		uint32 mipLevels,
		bool sRGB,
		bool autoDestroyImageBlob /*= true*/,
		const char* debugName /*= nullptr*/)
	{
		TextureCreateParams createParams;
		createParams.width                = imageBlob->width;
		createParams.height               = imageBlob->height;
		createParams.depth                = 1;
		createParams.mipLevels            = mipLevels;
		createParams.glDimension          = GL_TEXTURE_2D;
		createParams.glStorageFormat      = imageBlob->glStorageFormat;
		createParams.glPixelFormat        = imageBlob->glPixelFormat;
		createParams.glDataType           = imageBlob->glDataType;
		createParams.imageBlobs           = { imageBlob };
		createParams.autoDestroyImageBlob = autoDestroyImageBlob;

		if (debugName != nullptr) {
			createParams.debugName = debugName;
		}
		if (sRGB) {
			createParams.glStorageFormat = convertStorageFormatToSRGB(createParams.glStorageFormat);
		}

		Texture* texture = new Texture(createParams);
		texture->createGPUResource();
		return texture;
	}

	pathos::Texture* ImageUtils::createTextureCubeFromImages(
		std::vector<ImageBlob*> blobs,
		uint32 mipLevels,
		bool autoDestroyImageBlob /*= true*/,
		const char* debugName /*= nullptr*/)
	{
		for (int32 i = 0; i < 6; ++i) {
			if (blobs[i]->width != blobs[0]->width || blobs[i]->height != blobs[0]->width) {
				LOG(LogError, "%s: All cubemap faces must have same width and height", __FUNCTION__);
				return nullptr;
			}
		}

		TextureCreateParams createParams;
		createParams.width                = blobs[0]->width;
		createParams.height               = blobs[0]->height;
		createParams.depth                = 1;
		createParams.mipLevels            = mipLevels ? 0 : 1;
		createParams.glDimension          = GL_TEXTURE_CUBE_MAP;
		createParams.glStorageFormat      = blobs[0]->glStorageFormat;
		createParams.glPixelFormat        = blobs[0]->glPixelFormat;
		createParams.glDataType           = blobs[0]->glDataType;
		createParams.imageBlobs           = blobs;
		createParams.autoDestroyImageBlob = autoDestroyImageBlob;

		if (debugName != nullptr) {
			createParams.debugName = debugName;
		}

		Texture* texture = new Texture(createParams);
		texture->createGPUResource();
		return texture;
	}

	Texture* ImageUtils::createTexture3DFromImage(
		ImageBlob* imageBlob,
		const vector3ui& textureSize,
		uint32 mipLevels,
		bool sRGB,
		bool autoDestroyImageBlob /*= true*/,
		const char* debugName /*= nullptr*/)
	{
		TextureCreateParams createParams;
		createParams.width                = textureSize.x;
		createParams.height               = textureSize.y;
		createParams.depth                = textureSize.z;
		createParams.mipLevels            = mipLevels;
		createParams.glDimension          = GL_TEXTURE_3D;
		createParams.glStorageFormat      = imageBlob->glStorageFormat;
		createParams.glPixelFormat        = imageBlob->glPixelFormat;
		createParams.glDataType           = imageBlob->glDataType;
		createParams.imageBlobs           = { imageBlob };
		createParams.autoDestroyImageBlob = autoDestroyImageBlob;
		createParams.debugName            = debugName;

		if (sRGB) {
			createParams.glStorageFormat = convertStorageFormatToSRGB(createParams.glStorageFormat);
		}

		Texture* texture = new Texture(createParams);
		texture->createGPUResource();
		return texture;
	}

}

namespace pathos {

	void savePNG_RGB(int32 width, int32 height, uint8* blob, const char* filename) {
		FIBITMAP* dib = FreeImage_ConvertFromRawBits(
			blob, width, height, 3 * width, 24,
			0xff0000, // Red mask
			0x00ff00, // Green mask
			0x0000ff, // Blue mask
			false);
		FreeImage_Save(FIF_PNG, dib, filename, 0);
		FreeImage_Unload(dib);
	}

}
