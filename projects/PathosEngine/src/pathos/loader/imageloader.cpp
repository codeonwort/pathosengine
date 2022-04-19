#include "pathos/loader/imageloader.h"
#include "pathos/texture/volume_texture.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include "stb_image.h"

#pragma comment(lib, "FreeImage.lib")

// #todo-image-loader: Cleanup image loading API

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

	FIBITMAP* loadImage(const char* inFilename, bool flipHorizontal, bool flipVertical) {
		std::string path = ResourceFinder::get().find(inFilename);
		CHECK(path.size() != 0);

		LOG(LogDebug, "load image: %s", path.c_str());

		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path.c_str());
		FIBITMAP* img = FreeImage_Load(fif, path.c_str(), 0);

		if (!img) {
			LOG(LogError, "Error while loading: %s", path.c_str());
			return nullptr;
		}

		if (flipHorizontal) {
			FreeImage_FlipHorizontal(img);
		}
		if (flipVertical) {
			FreeImage_FlipVertical(img);
		}

		unsigned int bpp = FreeImage_GetBPP(img);
		if (bpp != 32 && bpp != 24) {
			FIBITMAP* img32 = FreeImage_ConvertTo32Bits(img);
			if (img32 != nullptr) {
				FreeImage_Unload(img);
				return img32;
			} else {
				FIBITMAP* img24 = FreeImage_ConvertTo24Bits(img);
				if (img24 != nullptr) {
					FreeImage_Unload(img);
					return img24;
				} else {
					FreeImage_Unload(img);
					CHECK_NO_ENTRY();
					return nullptr;
				}
			}
		}

		return img;
	}

	int32 loadCubemapImages(const std::array<const char*, 6>& inFilenames, ECubemapImagePreference preference, std::array<FIBITMAP*, 6>& outImages) {
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

	GLuint createTextureFromBitmap(FIBITMAP* dib, bool generateMipmap, bool sRGB, const char* debugName /*= nullptr*/) {
		int w, h;
		uint8* data;
		GLuint texture = 0;

		data = FreeImage_GetBits(dib);
		w = FreeImage_GetWidth(dib);
		h = FreeImage_GetHeight(dib);

		LOG(LogDebug, "%s: Create texture %dx%d", __FUNCTION__, w, h);

		ENQUEUE_RENDER_COMMAND([dib, data, w, h, texturePtr = &texture, generateMipmap, sRGB, debugName](RenderCommandList& cmdList) {
			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, texturePtr);
			if (debugName != nullptr) {
				gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugName);
			}

			uint32 numLODs = 1;
			if (generateMipmap) {
				numLODs = static_cast<uint32>(floor(log2(std::max(w, h))) + 1);
			}
			unsigned int bpp = FreeImage_GetBPP(dib);
			if (bpp == 32) {
				cmdList.textureStorage2D(*texturePtr, numLODs, sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8, w, h);
				cmdList.textureSubImage2D(*texturePtr, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, data);
			} else if (bpp == 24) {
				cmdList.textureStorage2D(*texturePtr, numLODs, sRGB ? GL_SRGB8 : GL_RGB8, w, h);
				cmdList.textureSubImage2D(*texturePtr, 0, 0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, data);
			} else {
				LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
				//gRenderDevice->deleteTextures(1, &tex_id);
			}
			if (generateMipmap) {
				cmdList.generateTextureMipmap(*texturePtr);
			}
		});

		// #todo-image-loader: dib is not guaranteed to be alive, so we should flush here for now.
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
	GLuint createCubemapTextureFromBitmap(FIBITMAP* dib[], bool generateMipmap /*= true*/, const char* debugName /*= nullptr*/) {
		int w = FreeImage_GetWidth(dib[0]);
		int h = FreeImage_GetHeight(dib[0]);
		if (w != h){
			LOG(LogError, "%s: Cubemap texture must have same width and height", __FUNCTION__);
			return 0;
		}

		GLuint tex_id = 0;

		ENQUEUE_RENDER_COMMAND([w, h, dib, generateMipmap, texturePtr = &tex_id, debugName](RenderCommandList& cmdList) {
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, texturePtr);
			if (debugName != nullptr) {
				gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugName);
			}

			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			unsigned int bpp = FreeImage_GetBPP(dib[0]);
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
				uint8* data = FreeImage_GetBits(dib[i]);
				GLenum format = bpp == 32 ? GL_BGRA : GL_BGR;
				cmdList.textureSubImage3D(*texturePtr, 0,
					0, 0, i,
					w, h, 1,
					format, GL_UNSIGNED_BYTE, data);
			}

			if (generateMipmap) {
				cmdList.generateTextureMipmap(*texturePtr);
			}
		});
		// #todo-image-loader: dib is not guaranteed to be alive, so we should flush here for now.
		TEMP_FLUSH_RENDER_COMMAND(true);

		return tex_id;
	}

	HDRImageMetadata loadHDRImage(const char* inFilename)
	{
		std::string path = ResourceFinder::get().find(inFilename);
		CHECK(path.size() != 0);

		int width, height, nrComponents;
		float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);

#if DEBUG_IMAGE_LOADER
		LOG(LogDebug, "loadHDRImage: %s (%dx%d)", path.c_str(), width, height);
#endif

		HDRImageMetadata metadata;
		metadata.data = data;
		metadata.width = width;
		metadata.height = height;

		return metadata;
	}

	GLuint createTextureFromHDRImage(const HDRImageMetadata& metadata, bool deleteBlobData /*= true*/, const char* debugName /*= nullptr*/)
	{
		static int32 debugNameAutoCounter = 0;

		GLuint texture = 0;

		ENQUEUE_RENDER_COMMAND([texturePtr = &texture, &metadata, debugName](RenderCommandList& cmdList) {
			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, texturePtr);

			if (debugName == nullptr) {
				char debugNameAuto[256];
				sprintf_s(debugNameAuto, "Texture HDR %d", debugNameAutoCounter);
				gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugNameAuto);
				debugNameAutoCounter += 1;
			} else {
				gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugName);
			}

			cmdList.textureStorage2D(*texturePtr, 1, GL_RGB16F, metadata.width, metadata.height);
			cmdList.textureSubImage2D(*texturePtr, 0, 0, 0, metadata.width, metadata.height, GL_RGB, GL_FLOAT, metadata.data);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_S, GL_REPEAT);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_WRAP_T, GL_REPEAT);
			//cmdList.textureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			//cmdList.textureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(*texturePtr, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		});
		// #todo-image-loader: metadata is not guaranteed to be alive, so we should flush GPU here.
		TEMP_FLUSH_RENDER_COMMAND(true);

		if (deleteBlobData) {
			stbi_image_free(metadata.data);
		}

		return texture;
	}

	void unloadHDRImage(const HDRImageMetadata& metadata)
	{
		stbi_image_free(metadata.data);
	}

	VolumeTexture* loadVolumeTextureFromTGA(const char* inFilename, const char* inDebugName)
	{
		// 1. Load data
		std::string path = ResourceFinder::get().find(inFilename);
		CHECK(path.size() != 0);

		LOG(LogDebug, "Load volume texture data: %s", path.c_str());

		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path.c_str());
		CHECKF(fif == FIF_TARGA, "Invalid Truevision Targa formats");

		FIBITMAP* img = FreeImage_Load(fif, path.c_str(), 0);

		if (!img) {
			LOG(LogError, "Error while loading: %s", path.c_str());
			return nullptr;
		}

		// 2. Create a volume texture
		VolumeTexture* vt = new VolumeTexture;
		vt->setImageData(img);
		vt->setDebugName(inDebugName);

		return vt;
	}

}
