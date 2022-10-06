#include "pathos/loader/imageloader.h"
#include "pathos/texture/volume_texture.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include "stb_image.h"

#include <FreeImage.h>

#pragma comment(lib, "FreeImage.lib")

// #todo-image-loader: Cleanup image loading API

namespace pathos {

	FIBITMAP* FreeImage_UnwrapBitmapBlob(const BitmapBlob* blob) {
		return reinterpret_cast<FIBITMAP*>(blob->fiHandle);
	}

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

	void initializeImageLibrary()
	{
		FreeImage_Initialise();
		LOG(LogInfo, "[ThirdParty] FreeImage %d.%d", FREEIMAGE_MAJOR_VERSION, FREEIMAGE_MINOR_VERSION);

		stbi_set_flip_vertically_on_load(true);
	}

	void destroyImageLibrary()
	{
		FreeImage_DeInitialise();
	}

	BitmapBlob* loadImage(const char* inFilename, bool flipHorizontal, bool flipVertical) {
		std::string path = ResourceFinder::get().find(inFilename);
		CHECK(path.size() != 0);

		LOG(LogDebug, "load image: %s", path.c_str());

		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path.c_str());
		FIBITMAP* dib = FreeImage_Load(fif, path.c_str(), 0);

		if (!dib) {
			LOG(LogError, "Error while loading: %s", path.c_str());
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

	/**
	* Generates cubemap texture from six faces.
	* 
	* Requirements
	* - Image order: +X, -X, +Y, -Y, +Z, -Z
	* - All images must have same width/height/bpp.
	*/
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

	HDRImageBlob* loadHDRImage(const char* inFilename)
	{
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

	VolumeTexture* loadVolumeTextureFromTGA(const char* inFilename, const char* inDebugName)
	{
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
		VolumeTexture* vt = new VolumeTexture;
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

}
