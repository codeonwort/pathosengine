#pragma once

#include "pathos/rhi/gl_handles.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	struct ImageBlob;
	struct BitmapBlob;

	struct TextureCreateParams {
		uint32 width              = 1;
		uint32 height             = 1;
		uint32 depth              = 1;
		uint32 mipLevels          = 1; // 1 = only lod0, 0 = generate all possible mipmaps, N = generate up to N mipmaps.
		GLenum glDimension        = 0; // ex) GL_TEXTURE_2D
		GLenum glStorageFormat    = 0; // ex) GL_RGBA8
		GLenum glPixelFormat      = 0; // ex) GL_RGBA
		GLenum glDataType         = 0; // ex) GL_FLOAT
		ImageBlob* imageBlob      = nullptr;
		bool autoDestroyImageBlob = true;
		std::string debugName;

		inline bool isValid() const {
			return width > 0 && height > 0 && depth > 0 && glDimension != 0 && glStorageFormat != 0 && glPixelFormat != 0 && glDataType != 0;
		}
	};

	// A wrapper for GPU texture resource.
	// #wip: Turn this into a generalized Texture class. Use ImageBlob instead of BitmapBlob.
	class Texture final : public Noncopyable {

	public:
		Texture(const TextureCreateParams& inCreateParams)
			: createParams(inCreateParams)
		{}

		~Texture();

		void createGPUResource(bool flushGPU = false);

		inline const TextureCreateParams& getCreateParams() const { return createParams; }
		inline GLuint internal_getGLName() const { return glTexture; }

		//~ BEGIN old API
		void setBitmapData(BitmapBlob* inData);
		void initGLResource(uint32 textureWidth, uint32 textureHeight, uint32 textureDepth);
		
		inline bool isValid() const { return glTexture != 0; }
		inline uint32 getWidth() const { return createParams.width; }
		inline uint32 getHeight() const { return createParams.height; }
		//~ END old API

	private:
		const TextureCreateParams createParams;
		GLuint glTexture = 0;
		bool created = false;

		BitmapBlob* bitmapInfo = nullptr;

	};

}
