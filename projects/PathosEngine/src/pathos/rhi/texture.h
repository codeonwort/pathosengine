#pragma once

#include "pathos/rhi/gl_handles.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	struct ImageBlob;

	struct TextureCreateParams {
		uint32 width              = 1;
		uint32 height             = 1;
		uint32 depth              = 1;
		uint32 mipLevels          = 1; // 1 = only lod0, 0 = generate all possible mipmaps, N = generate up to N mipmaps.
		GLenum glDimension        = 0; // ex) GL_TEXTURE_2D
		GLenum glStorageFormat    = 0; // ex) GL_RGBA8

		// #wip: pixel format and data type are meaningful only if imageBlob exists.
		GLenum glPixelFormat      = 0; // ex) GL_RGBA
		GLenum glDataType         = 0; // ex) GL_FLOAT
		bool autoDestroyImageBlob = true;
		std::vector<ImageBlob*> imageBlobs;

		std::string debugName;

		inline bool isValid() const {
			bool valid = width > 0 && height > 0 && depth > 0 && glDimension != 0 && glStorageFormat != 0;
			if (imageBlobs.size() > 0) {
				valid = valid && glPixelFormat != 0 && glDataType != 0;
			}
			return valid;
		}
	};

	// Wrapper for GPU texture resource.
	class Texture final : public Noncopyable {

	public:
		// Call createGPUResource() to actually allocate a GL texture in VRAM.
		Texture(const TextureCreateParams& inCreateParams)
			: createParams(inCreateParams)
		{}

		~Texture();

		void createGPUResource(bool flushGPU = false);
		void releaseGPUResource();

		inline const TextureCreateParams& getCreateParams() const { return createParams; }
		inline GLuint internal_getGLName() const { return glTexture; }
		inline bool isValid() const { return glTexture != 0; }

	private:
		const TextureCreateParams createParams;
		GLuint glTexture = 0;
		bool created = false;

	};

}
