#pragma once

#include "pathos/rhi/gl_handles.h"
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

		// Call if the GL texture should be visible to main thread immediately.
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

	class RenderTargetCube final {

	public:
		RenderTargetCube() = default;
		~RenderTargetCube();

		// Cubemap width = height
		void respecTexture(uint32 inWidth, RenderTargetFormat inFormat, const char* inDebugName = nullptr);

		// Call if the GL texture should be visible to main thread immediately.
		void immediateUpdateResources();

		void destroyResources();

		inline uint32 getWidth() const { return width; }

		inline GLuint getGLName() const { return glTextureObject; }

		bool isTextureValid() const;
		bool isColorFormat() const;
		bool isDepthFormat() const;

	private:
		GLuint glTextureObject = 0;
		GLuint glTextureViews[6] = { 0, };
		uint32 width = 0;
		RenderTargetFormat format = RenderTargetFormat::RGBA16F;

	};

}
