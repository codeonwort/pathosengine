#pragma once

#include "badger/types/int_types.h"
#include "gl_core.h"

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

		void initializeResource();
		void destroyResource();

		void respecTexture(uint32 inWidth, uint32 inHeight, RenderTargetFormat inFormat);

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline GLuint getGLName() const { return glTextureObject; }

	private:
		// Returns true if GL texture is invalid or its current spec doesn't match with the render target spec.
		bool isTextureInvalid() const;

		GLuint glTextureObject = 0;
		uint32 width;
		uint32 height;
		RenderTargetFormat format;

	};

}
