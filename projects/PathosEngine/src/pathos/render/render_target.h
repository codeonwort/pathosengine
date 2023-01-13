#pragma once

#include "pathos/rhi/gl_handles.h"
#include "badger/types/int_types.h"

namespace pathos {

	class RenderTarget2D;
	class RenderTargetCube;

	enum class RenderTargetFormat : uint8
	{
		R16F,

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

	class RenderTargetView final {
		
	public:
		RenderTargetView(RenderTarget2D* inRenderTarget2D) {
			renderTarget2D = inRenderTarget2D;
			layer = 0;
		}

		RenderTargetView(RenderTargetCube* inRenderTargetCube, uint32 inLayer) {
			renderTargetCube = inRenderTargetCube;
			layer = inLayer;
			CHECK(0 <= layer && layer < 6);
		}

		GLuint getGLName() const;
		bool isDepthFormat() const;

	private:
		RenderTarget2D* renderTarget2D = nullptr;
		RenderTargetCube* renderTargetCube = nullptr;
		uint32 layer;

	};

	// Wrapper for a 2D texture which can be used as a render target.
	class RenderTarget2D final {

	public:
		RenderTarget2D();
		~RenderTarget2D();

		void respecTexture(
			uint32 inWidth,
			uint32 inHeight,
			RenderTargetFormat inFormat,
			const char* inDebugName = nullptr);

		// Call if the GL texture should be visible to main thread immediately.
		void immediateUpdateResource();

		void destroyResource();

		RenderTargetView* getRenderTargetView() const;

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
		uniquePtr<RenderTargetView> renderTargetView;

	};

	class RenderTargetCube final {

	public:
		RenderTargetCube();
		~RenderTargetCube();

		// Cubemap width = height
		void respecTexture(
			uint32 inWidth,
			RenderTargetFormat inFormat,
			uint32 inNumMips, // 0 = full mips
			const char* inDebugName = nullptr);

		// Call if the GL texture should be visible to main thread immediately.
		void immediateUpdateResources();

		void destroyResources();

		RenderTargetView* getRenderTargetView(uint32 faceIndex) const;

		inline uint32 getWidth() const { return width; }
		inline uint32 getNumMips() const { return numMips; }

		inline GLuint getGLTexture() const { return glTextureObject; }
		inline GLuint getGLTextureView(uint32 faceIndex) { return glTextureViews[faceIndex]; }

		bool isTextureValid() const;
		bool isColorFormat() const;
		bool isDepthFormat() const;

	private:
		GLuint glTextureObject = 0;
		GLuint glTextureViews[6] = { 0, };
		uint32 width = 0;
		uint32 numMips = 0;
		RenderTargetFormat format = RenderTargetFormat::RGBA16F;
		uniquePtr<RenderTargetView> renderTargetViews[6];

	};

}
