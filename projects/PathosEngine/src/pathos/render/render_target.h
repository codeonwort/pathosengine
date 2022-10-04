#pragma once

#include "pathos/gl_handles.h"
#include "badger/types/int_types.h"

namespace pathos {

	enum class RenderTargetFormat : uint8
	{
		RGBA16F,
		RGBA16I,
		RGBA16UI,

		RGBA32F,
		RGBA32I,
		RGBA32UI,

		DEPTH16F,
		DEPTH24F,
		DEPTH32F,
	};

	// Wrapper for a 2D texture which can be used as a render target.
	class RenderTarget2D final {

	public:
		RenderTarget2D() = default;
		~RenderTarget2D();

		void respecTexture(uint32 inWidth, uint32 inHeight, RenderTargetFormat inFormat);
		void immediateUpdateResource();
		void destroyResource();

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline GLuint getGLName() const { return glTextureObject; }

		bool isTextureValid() const;
		bool isColorFormat() const;
		bool isDepthFormat() const;

	private:
		GLuint glTextureObject = 0;
		uint32 width = 0;
		uint32 height = 0;
		RenderTargetFormat format = RenderTargetFormat::RGBA16F;

	};

}
