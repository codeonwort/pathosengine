#include "pathos/loader/image_loader.h"

#include "pathos/rhi/texture.h"
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

	FIBITMAP* FreeImage_UnwrapBitmapBlob(const BitmapBlob* blob) {
		return reinterpret_cast<FIBITMAP*>(blob->fiHandle);
	}

	void FreeImage_GetGLFormats(
		FREE_IMAGE_FORMAT freeImageFormat, uint32 bpp,
		GLenum& outStorageFormat, GLenum& outPixelFormat, GLenum& outDataType)
	{
		switch (freeImageFormat) {
			case FIF_BMP:
			case FIF_JPEG:
			case FIF_PNG:
				outStorageFormat = (bpp == 32) ? GL_RGBA8 : (bpp == 24) ? GL_RGB8 : GL_NONE;
				outPixelFormat = (bpp == 32) ? GL_BGRA : (bpp == 24) ? GL_BGR : GL_NONE;
				outDataType = GL_UNSIGNED_BYTE;
				break;
			case FIF_HDR:
			case FIF_EXR:
				// #wip: What if HDR format only contains RGB data?
				outStorageFormat = (bpp == 128) ? GL_RGBA32F : (bpp == 64) ? GL_RGBA16F : GL_NONE;
				outPixelFormat = (bpp == 128 || bpp == 64) ? GL_RGBA : GL_NONE;
				outDataType = GL_FLOAT;
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
			case FIF_TARGA:
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

		BYTE* rawData = FreeImage_GetBits(dib);
		unsigned int width = FreeImage_GetWidth(dib);
		unsigned int height = FreeImage_GetHeight(dib);
		unsigned int bpp = FreeImage_GetBPP(dib);

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

	GLuint ImageUtils::createTextureFromImage(
		ImageBlob* imageBlob,
		bool generateMipmaps,
		bool sRGB,
		bool autoDestroyImageBlob /*= true*/,
		const char* debugName /*= nullptr*/)
	{
		GLuint texture = 0;

		ENQUEUE_RENDER_COMMAND(
			[imageBlob,
			texturePtr = &texture, generateMipmaps, sRGB,
			debugName, autoDestroyImageBlob](RenderCommandList& cmdList)
			{
				gRenderDevice->createTextures(GL_TEXTURE_2D, 1, texturePtr);
				if (debugName != nullptr) {
					gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugName);
				}

				uint32 width = imageBlob->width;
				uint32 height = imageBlob->height;
				uint32 bpp = imageBlob->bpp;
				uint8* rawBytes = imageBlob->rawBytes;

				GLenum storageFormat = imageBlob->glStorageFormat;
				GLenum pixelFormat = imageBlob->glPixelFormat;
				GLenum dataType = imageBlob->glDataType;
				if (sRGB) {
					if (storageFormat == GL_RGBA8) storageFormat = GL_SRGB8_ALPHA8;
					else if (storageFormat == GL_RGB8) storageFormat = GL_SRGB8;
				}

				uint32 numLODs = generateMipmaps ? static_cast<uint32>(floor(log2(std::max(width, height))) + 1) : 1;

				cmdList.textureStorage2D(*texturePtr, numLODs, storageFormat, width, height);
				cmdList.textureSubImage2D(*texturePtr, 0, 0, 0, width, height, pixelFormat, dataType, rawBytes);

				if (generateMipmaps) {
					cmdList.generateTextureMipmap(*texturePtr);
				}
				if (autoDestroyImageBlob) {
					cmdList.registerDeferredCleanup(imageBlob);
				}
			});

		// #todo-image-loader: There is no wrapper for 'texture', so we should flush to finalize it.
		TEMP_FLUSH_RENDER_COMMAND(true);

		return texture;
	}

}

namespace pathos {

	BitmapBlob::BitmapBlob(void* inFIBITMAP, bool inHasOpacity) {
		fiHandle = inFIBITMAP;
		hasOpacity = inHasOpacity;
		if (inFIBITMAP != nullptr) {
			FIBITMAP* dib = FreeImage_UnwrapBitmapBlob(this);
			width = FreeImage_GetWidth(dib);
			height = FreeImage_GetHeight(dib);
			bpp = FreeImage_GetBPP(dib);
		}
	}

	BitmapBlob::BitmapBlob(uint8* inRawBytes, uint32 inWidth, uint32 inHeight, uint32 inBpp, bool inHasOpacity) {
		CHECK(inRawBytes != nullptr && inWidth > 0 && inHeight > 0 && inBpp > 0);
		externalRawBytes = inRawBytes;
		width = inWidth;
		height = inHeight;
		bpp = inBpp;
		hasOpacity = inHasOpacity;

		fiHandle = nullptr;
	}

	BitmapBlob::~BitmapBlob() {
		if (fiHandle) {
			FreeImage_Unload((FIBITMAP*)fiHandle);
			fiHandle = nullptr;
		}
	}

	uint8* BitmapBlob::getRawBytes() const {
		if (fiHandle != nullptr) {
			FIBITMAP* dib = FreeImage_UnwrapBitmapBlob(this);
			return reinterpret_cast<uint8*>(FreeImage_GetBits(dib));
		}
		return externalRawBytes;
	}

	HDRImageBlob::~HDRImageBlob() {
		if (rawData) {
			stbi_image_free(rawData);
			rawData = nullptr;
		}
	}

}

namespace pathos {

	BitmapBlob* loadImage(const char* inFilename, bool flipHorizontal, bool flipVertical) {
		std::string path = ResourceFinder::get().find(inFilename);
		if (path.size() == 0) {
			LOG(LogError, "[loadImage] Can't find: %s", inFilename);
			return nullptr;
		}

		//LOG(LogDebug, "load image: %s", path.c_str());

		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path.c_str());
		FIBITMAP* dib = FreeImage_Load(fif, path.c_str(), 0);

		if (!dib) {
			LOG(LogError, "[loadImage] Error while loading: %s", path.c_str());
			return nullptr;
		}

		if (flipHorizontal) {
			FreeImage_FlipHorizontal(dib);
		}
		if (flipVertical) {
			FreeImage_FlipVertical(dib);
		}

		unsigned int bpp = FreeImage_GetBPP(dib);
		const bool hasOpacity = (bpp == 32);

		if (bpp != 32 && bpp != 24) {
			FIBITMAP* dib32 = FreeImage_ConvertTo32Bits(dib);
			if (dib32 != nullptr) {
				FreeImage_Unload(dib);
				dib = dib32;
				bpp = 32;
			} else {
				FIBITMAP* dib24 = FreeImage_ConvertTo24Bits(dib);
				if (dib24 != nullptr) {
					FreeImage_Unload(dib);
					dib = dib24;
				} else {
					FreeImage_Unload(dib);
					CHECK_NO_ENTRY();
					return nullptr;
				}
			}
		}

		BitmapBlob* blob = new BitmapBlob(dib, hasOpacity);
		blob->bIsBGR = true;

		return blob;
	}

	int32 loadCubemapImages(
		const std::array<const char*, 6>& inFilenames,
		ECubemapImagePreference preference,
		std::array<BitmapBlob*, 6>& outImages)
	{
		int32 ret = 0;
		const int32 glslOrder[6] = { 0, 1, 2, 3, 5, 4 };

		for (int32 i = 0; i < 6; ++i) {
			bool flipH = preference == ECubemapImagePreference::HLSL && (i != 2 && i != 3);
			bool flipV = preference == ECubemapImagePreference::HLSL && (i != 2 && i != 3);
			int32 j = preference == ECubemapImagePreference::HLSL ? glslOrder[i] : i;
			outImages[i] = loadImage(inFilenames[j], flipH, flipV);

			if(outImages[i] != nullptr) {
				ret += 1;
			}
		}

		return ret;
	}

	HDRImageBlob* loadHDRImage(const char* inFilename) {
		std::string path = ResourceFinder::get().find(inFilename);
		CHECK(path.size() != 0);

		int width, height, nrComponents;
		float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);

#if DEBUG_IMAGE_LOADER
		LOG(LogDebug, "loadHDRImage: %s (%dx%d)", path.c_str(), width, height);
#endif

		HDRImageBlob* blob = new HDRImageBlob;
		blob->rawData = data;
		blob->width = width;
		blob->height = height;

		return blob;
	}

	Texture* loadVolumeTextureFromTGA(const char* inFilename, const char* inDebugName) {
		// 1. Load data
		std::string path = ResourceFinder::get().find(inFilename);
		CHECK(path.size() != 0);

		LOG(LogDebug, "Load volume texture data: %s", path.c_str());

		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path.c_str());
		CHECKF(fif == FIF_TARGA, "Invalid Truevision Targa formats");

		FIBITMAP* dib = FreeImage_Load(fif, path.c_str(), 0);

		if (!dib) {
			LOG(LogError, "Error while loading: %s", path.c_str());
			return nullptr;
		}

		// 2. Create a volume texture
		Texture* vt = new Texture;
		vt->setImageData(new BitmapBlob(dib));
		vt->setDebugName(inDebugName);

		return vt;
	}

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

	GLuint createTextureFromBitmap(
		BitmapBlob* bitmapBlob,
		bool generateMipmap,
		bool sRGB,
		const char* debugName /*= nullptr*/,
		bool autoDestroyBlob /*= true*/)
	{
		GLuint texture = 0;

		//LOG(LogDebug, "%s: Create texture %ux%u", __FUNCTION__, bitmapBlob->width, bitmapBlob->height);

		ENQUEUE_RENDER_COMMAND(
			[bitmapBlob,
			texturePtr = &texture, generateMipmap, sRGB,
			debugName, autoDestroyBlob](RenderCommandList& cmdList)
		{
			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, texturePtr);
			if (debugName != nullptr) {
				gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugName);
			}

			uint32 w = bitmapBlob->width;
			uint32 h = bitmapBlob->height;
			uint32 bpp = bitmapBlob->bpp;
			uint8* rawBytes = bitmapBlob->getRawBytes();

			uint32 numLODs = 1;
			if (generateMipmap) {
				numLODs = static_cast<uint32>(floor(log2(std::max(w, h))) + 1);
			}
			
			if (bpp == 32) {
				GLenum fmt = bitmapBlob->bIsBGR ? GL_BGRA : GL_RGBA;
				cmdList.textureStorage2D(*texturePtr, numLODs, sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8, w, h);
				cmdList.textureSubImage2D(*texturePtr, 0, 0, 0, w, h, fmt, GL_UNSIGNED_BYTE, rawBytes);
			} else if (bpp == 24) {
				GLenum fmt = bitmapBlob->bIsBGR ? GL_BGR : GL_RGB;
				cmdList.textureStorage2D(*texturePtr, numLODs, sRGB ? GL_SRGB8 : GL_RGB8, w, h);
				cmdList.textureSubImage2D(*texturePtr, 0, 0, 0, w, h, fmt, GL_UNSIGNED_BYTE, rawBytes);
			} else {
				LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
				//gRenderDevice->deleteTextures(1, &tex_id);
			}
			if (generateMipmap) {
				cmdList.generateTextureMipmap(*texturePtr);
			}

			if (autoDestroyBlob) {
				cmdList.registerDeferredCleanup(bitmapBlob);
			}
		});

		// #todo-image-loader: There is no wrapper for 'texture', so we should flush to finalize it.
		TEMP_FLUSH_RENDER_COMMAND(true);

		return texture;
	}

	GLuint createCubemapTextureFromBitmap(
		BitmapBlob* blobs[],
		bool generateMipmap /*= true*/,
		const char* debugName /*= nullptr*/,
		bool autoDestroyBlob /*= true*/)
	{
		for (int32 i = 0; i < 6; ++i) {
			if (blobs[i]->width != blobs[0]->width || blobs[i]->height != blobs[0]->width) {
				LOG(LogError, "%s: All cubemap faces must have same width and height", __FUNCTION__);
				return 0;
			}
		}

		GLuint tex_id = 0;

		std::array<BitmapBlob*, 6> blobArray;
		for (size_t i = 0u; i < 6; ++i) blobArray[i] = blobs[i];

		ENQUEUE_RENDER_COMMAND([blobArray, generateMipmap, texturePtr = &tex_id, debugName, autoDestroyBlob](RenderCommandList& cmdList) {
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, texturePtr);
			if (debugName != nullptr) {
				gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugName);
			}

			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			uint32 w = blobArray[0]->width;
			uint32 h = blobArray[0]->height;
			uint32 bpp = blobArray[0]->bpp;

			if (bpp == 32 || bpp == 24) {
				uint32 numLODs = 1;
				if (generateMipmap) {
					numLODs = static_cast<uint32>(floor(log2(std::max(w, h))) + 1);
				}
				cmdList.textureStorage2D(*texturePtr, numLODs, GL_RGBA8, w, h);
			} else {
				LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
			}

			for (int32 i = 0; i < 6; i++) {
				uint8* data = blobArray[i]->getRawBytes();
				GLenum format = blobArray[i]->bIsBGR
					? ((bpp == 32) ? GL_BGRA : GL_BGR)
					: ((bpp == 32) ? GL_RGBA : GL_RGB);
				cmdList.textureSubImage3D(*texturePtr, 0,
					0, 0, i,
					w, h, 1,
					format, GL_UNSIGNED_BYTE, data);
			}

			if (generateMipmap) {
				cmdList.generateTextureMipmap(*texturePtr);
			}
			if (autoDestroyBlob) {
				for (int32 i = 0; i < 6; ++i) {
					cmdList.registerDeferredCleanup(blobArray[i]);
				}
			}
		});

		// #todo-image-loader: There is no wrapper for 'texture', so we should flush to finalize it.
		TEMP_FLUSH_RENDER_COMMAND(true);

		return tex_id;
	}

	GLuint createTextureFromHDRImage(
		HDRImageBlob* blob,
		bool deleteBlobData /*= true*/,
		const char* debugName /*= nullptr*/)
	{
		static int32 debugNameAutoCounter = 0;

		GLuint texture = 0;

		ENQUEUE_RENDER_COMMAND([texturePtr = &texture, blob, debugName, deleteBlobData](RenderCommandList& cmdList) {
			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, texturePtr);

			if (debugName == nullptr) {
				char debugNameAuto[256];
				sprintf_s(debugNameAuto, "Texture HDR %d", debugNameAutoCounter);
				gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugNameAuto);
				debugNameAutoCounter += 1;
			} else {
				gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugName);
			}

			cmdList.textureStorage2D(*texturePtr, 1, GL_RGB16F, blob->width, blob->height);
			cmdList.textureSubImage2D(*texturePtr, 0, 0, 0, blob->width, blob->height, GL_RGB, GL_FLOAT, blob->rawData);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_S, GL_REPEAT);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_T, GL_REPEAT);
			//cmdList.textureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			//cmdList.textureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (deleteBlobData) {
				cmdList.registerDeferredCleanup(blob);
			}
		});

		// #todo-image-loader: There is no wrapper for 'texture', so we should flush to finalize it.
		TEMP_FLUSH_RENDER_COMMAND(true);

		return texture;
	}

}
