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
		GLenum glStorageFormat    = 0; // ex) GL_RGBA16F
		bool autoDestroyImageBlob = true;
		std::vector<ImageBlob*> imageBlobs;
		std::string debugName;

		// NOTE: Does not check if image blob is valid.
		inline bool isStorageValid() const {
			return width > 0 && height > 0 && depth > 0 && glDimension != 0 && glStorageFormat != 0;
		}

		static TextureCreateParams cubemap(uint32 size, GLenum glStorageFormat, uint32 mipLevels = 1) {
			return TextureCreateParams{ size, size,1, mipLevels, GL_TEXTURE_CUBE_MAP, glStorageFormat };
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

		void createGPUResource_renderThread(RenderCommandList& cmdList);

		inline const TextureCreateParams& getCreateParams() const { return createParams; }
		inline GLuint internal_getGLName() const { return glTexture; }

		// Game logic can assume that this texture have a valid GL texture.
		inline bool isCreated() const { return created; }

	private:
		const TextureCreateParams createParams;
		GLuint glTexture = 0;
		bool created = false;

	};

}
