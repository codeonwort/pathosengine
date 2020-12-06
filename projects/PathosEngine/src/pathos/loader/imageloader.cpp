#include "pathos/loader/imageloader.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"
#include "pathos/texture/volume_texture.h"

#include "badger/assertion/assertion.h"
#include "stb_image.h"

#pragma comment(lib, "FreeImage.lib")

// #todo-image-loader: Cleanup image loading API
// #todo-image-loader: Remove direct GL calls

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

	GLuint createTextureFromBitmap(FIBITMAP* dib, bool generateMipmap, bool sRGB) {
		int w, h;
		uint8* data;
		GLuint tex_id = 0;

		data = FreeImage_GetBits(dib);
		w = FreeImage_GetWidth(dib);
		h = FreeImage_GetHeight(dib);

		LOG(LogDebug, "%s: Create texture %dx%d", __FUNCTION__, w, h);

		glCreateTextures(GL_TEXTURE_2D, 1, &tex_id);

		uint32 numLODs = 1;
		if (generateMipmap) {
			numLODs = static_cast<uint32>(floor(log2(std::max(w, h))) + 1);
		}
		unsigned int bpp = FreeImage_GetBPP(dib);
		if (bpp == 32) {
			if (sRGB) {
				glTextureStorage2D(tex_id, numLODs, GL_SRGB8_ALPHA8, w, h);
			} else {
				glTextureStorage2D(tex_id, numLODs, GL_RGBA8, w, h);
			}
			glTextureSubImage2D(tex_id, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, data);
		} else if (bpp == 24) {
			if (sRGB) {
				glTextureStorage2D(tex_id, numLODs, GL_SRGB8, w, h);
			} else {
				glTextureStorage2D(tex_id, numLODs, GL_RGBA8, w, h);
			}
			glTextureSubImage2D(tex_id, 0, 0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, data);
		} else {
#ifdef _DEBUG
			LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
#endif
			glDeleteTextures(1, &tex_id);
			return 0;
		}
		if (generateMipmap) {
			glGenerateTextureMipmap(tex_id);
		}

		return tex_id;
	}

	/**
	* Generates cubemap texture from six faces.
	* 
	* Requirements
	* - Image order: +X, -X, +Y, -Y, +Z, -Z
	* - All images must have same width/height/bpp.
	*/
	GLuint createCubemapTextureFromBitmap(FIBITMAP* dib[], bool generateMipmap) {
		int w = FreeImage_GetWidth(dib[0]);
		int h = FreeImage_GetHeight(dib[0]);
		if (w != h){
			LOG(LogError, "%s: Cubemap texture must have same width and height", __FUNCTION__);
			return 0;
		}

		GLuint tex_id;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &tex_id);
		glTextureParameteri(tex_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTextureParameteri(tex_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(tex_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(tex_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		unsigned int bpp = FreeImage_GetBPP(dib[0]);
		if (bpp == 32 || bpp == 24) {
			uint32 numLODs = 1;
			if (generateMipmap) {
				numLODs = static_cast<uint32>(floor(log2(std::max(w, h))) + 1);
			}
			glTextureStorage2D(tex_id, numLODs, GL_RGBA8, w, h);
		} else {
			LOG(LogError, "%s: Unexpected BPP = %d", __FUNCTION__, bpp);
			return 0;
		}

		for (int32 i = 0; i < 6; i++) {
			uint8* data = FreeImage_GetBits(dib[i]);
			GLenum format = bpp == 32 ? GL_BGRA : GL_BGR;
			glTextureSubImage3D(tex_id, 0,
				0, 0, i,
				w, h, 1,
				format, GL_UNSIGNED_BYTE, data);
		}

		if (generateMipmap) {
			glGenerateTextureMipmap(tex_id);
		}

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

	GLuint createTextureFromHDRImage(const HDRImageMetadata& metadata, bool deleteBlobData /*= true*/)
	{
		static int32 label_counter = 0;

		GLuint texture;
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);
		glTextureStorage2D(texture, 1, GL_RGB16F, metadata.width, metadata.height);
		glTextureSubImage2D(texture, 0, 0, 0, metadata.width, metadata.height, GL_RGB, GL_FLOAT, metadata.data);
		glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		char label[256];
		sprintf_s(label, "Texture HDR %d", label_counter);
		glObjectLabel(GL_TEXTURE, texture, -1, label);
		label_counter += 1;

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
