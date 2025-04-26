#include "render_target.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/texture.h"

namespace pathos {

	GLenum RENDER_TARGET_FORMAT_TO_GL_FORMAT(RenderTargetFormat RTF) {
		switch (RTF) {
			case RenderTargetFormat::R8F:          return GL_R8;
			case RenderTargetFormat::R16F:         return GL_R16F;
			case RenderTargetFormat::RGBA16F:      return GL_RGBA16F;
			case RenderTargetFormat::RGBA16I:      return GL_RGBA16I;
			case RenderTargetFormat::RGBA16UI:     return GL_RGBA16UI;
			case RenderTargetFormat::RGBA32F:      return GL_RGBA32F;
			case RenderTargetFormat::RGBA32I:      return GL_RGBA32I;
			case RenderTargetFormat::RGBA32UI:     return GL_RGBA32UI;
			case RenderTargetFormat::DEPTH16F:     return GL_DEPTH_COMPONENT16;
			case RenderTargetFormat::DEPTH24F:     return GL_DEPTH_COMPONENT24;
			case RenderTargetFormat::DEPTH32F:     return GL_DEPTH_COMPONENT32F;
			default:
				CHECKF(0, "Missing case");
				break;
			}
		return 0;
	}

	bool RENDER_TARGET_FORMAT_isColorFormat(RenderTargetFormat format) {
		switch (format) {
			case RenderTargetFormat::R8F:
			case RenderTargetFormat::R16F:
			case RenderTargetFormat::RGBA16F:
			case RenderTargetFormat::RGBA16I:
			case RenderTargetFormat::RGBA16UI:
			case RenderTargetFormat::RGBA32F:
			case RenderTargetFormat::RGBA32I:
			case RenderTargetFormat::RGBA32UI:
				return true;
			case RenderTargetFormat::DEPTH16F:
			case RenderTargetFormat::DEPTH24F:
			case RenderTargetFormat::DEPTH32F:
				return false;
			default:
				CHECKF(0, "Missing case");
				break;
		}
		return false;
	}

	bool RENDER_TARGET_FORMAT_isDepthFormat(RenderTargetFormat format) {
		switch (format) {
			case RenderTargetFormat::R8F:
			case RenderTargetFormat::R16F:
			case RenderTargetFormat::RGBA16F:
			case RenderTargetFormat::RGBA16I:
			case RenderTargetFormat::RGBA16UI:
			case RenderTargetFormat::RGBA32F:
			case RenderTargetFormat::RGBA32I:
			case RenderTargetFormat::RGBA32UI:
				return false;
			case RenderTargetFormat::DEPTH16F:
			case RenderTargetFormat::DEPTH24F:
			case RenderTargetFormat::DEPTH32F:
				return true;
			default:
				CHECKF(0, "Missing case");
				break;
		}
		return false;
	}

}

// RenderTargetView
namespace pathos {

	GLuint RenderTargetView::getGLName() const {
		if (renderTarget2D != nullptr) {
			return renderTarget2D->getInternalTexture()->internal_getGLName();
		} else {
			return renderTargetCube->getGLTextureView(layer);
		}
	}

	bool RenderTargetView::isDepthFormat() const {
		if (renderTarget2D != nullptr) {
			return renderTarget2D->isDepthFormat();
		} else {
			return renderTargetCube->isDepthFormat();
		}
	}

}

// RenderTarget2D
namespace pathos {

	RenderTarget2D::RenderTarget2D() {
		renderTargetView = makeUnique<RenderTargetView>(this);
	}

	RenderTarget2D::~RenderTarget2D() {
		destroyResource();
	}

	void RenderTarget2D::respecTexture(
		uint32 inWidth,
		uint32 inHeight,
		RenderTargetFormat inFormat,
		const char* inDebugName)
	{
		const bool validDimension = inWidth != 0 && inHeight != 0;
		CHECKF(validDimension, "Invalid width or height");

		if (!validDimension) {
			return;
		}

		destroyResource();
		format = inFormat;

		TextureCreateParams createParams;
		createParams.width = inWidth;
		createParams.height = inHeight;
		createParams.depth = 1;
		createParams.mipLevels = 1;
		createParams.glDimension = GL_TEXTURE_2D;
		createParams.glStorageFormat = RENDER_TARGET_FORMAT_TO_GL_FORMAT(format);
		if (inDebugName != nullptr) createParams.debugName = inDebugName;

		texture = new Texture(createParams);
		texture->createGPUResource();
	}

	void RenderTarget2D::immediateUpdateResource() {
		FLUSH_RENDER_COMMAND();
	}

	void RenderTarget2D::destroyResource() {
		if (texture != nullptr) {
			delete texture;
			texture = nullptr;
		}
	}

	RenderTargetView* RenderTarget2D::getRenderTargetView() const {
		return renderTargetView.get();
	}

	uint32 RenderTarget2D::getWidth() const {
		return texture->getCreateParams().width;
	}

	uint32 RenderTarget2D::getHeight() const {
		return texture->getCreateParams().height;
	}

	bool RenderTarget2D::isTextureValid() const {
		return texture != nullptr && texture->isCreated();
	}

	bool RenderTarget2D::isColorFormat() const {
		return RENDER_TARGET_FORMAT_isColorFormat(format);
	}

	bool RenderTarget2D::isDepthFormat() const {
		return RENDER_TARGET_FORMAT_isDepthFormat(format);
	}

}

// RenderTargetCube
namespace pathos {

	RenderTargetCube::RenderTargetCube() {
		for (uint32 i = 0; i < 6; ++i) {
			renderTargetViews[i] = makeUnique<RenderTargetView>(this, i);
		}
	}

	RenderTargetCube::~RenderTargetCube() {
		destroyResources();
	}

	void RenderTargetCube::respecTexture(
		uint32 inWidth,
		RenderTargetFormat inFormat,
		uint32 inNumMips,
		const char* inDebugName)
	{
		const bool bValidDimension = (inWidth != 0);
		CHECKF(bValidDimension, "Invalid width");

		if (!bValidDimension) {
			return;
		}

		destroyResources();
		format = inFormat;

		const GLenum glFormat = RENDER_TARGET_FORMAT_TO_GL_FORMAT(format);
		uint32 fullMips = (uint32)(floor(log2(inWidth)) + 1);
		uint32 numMips = (inNumMips == 0) ? fullMips : std::min(inNumMips, fullMips);

		TextureCreateParams createParams = TextureCreateParams::cubemap(inWidth, glFormat, numMips);
		if (inDebugName != nullptr) createParams.debugName = inDebugName;

		texture = new Texture(createParams);

		Texture* texturePtr = texture;
		GLuint* textureViewsPtr = glTextureViews;
		std::string debugName = inDebugName ? inDebugName : std::string();

		ENQUEUE_RENDER_COMMAND(
			[texturePtr, textureViewsPtr, debugName](RenderCommandList& cmdList) {
				texturePtr->createGPUResource_renderThread(cmdList);

				// Texture views
				gRenderDevice->genTextures(6, textureViewsPtr);
				for (uint32 i = 0; i < 6; ++i) {
					GLuint view = textureViewsPtr[i];
					cmdList.textureView(
						view, GL_TEXTURE_2D, texturePtr->internal_getGLName(), texturePtr->getCreateParams().glStorageFormat,
						0, 1, // LOD0
						i, 1  // layer[i]
					);
					if (debugName.size() > 0) {
						std::string viewName = debugName + "_view" + std::to_string(i);
						cmdList.objectLabel(GL_TEXTURE, view, -1, viewName.c_str());
					}
				}
			}
		);
	}

	void RenderTargetCube::immediateUpdateResources() {
		FLUSH_RENDER_COMMAND();
	}

	void RenderTargetCube::destroyResources() {
		if (texture != nullptr) {
			delete texture;
			texture = nullptr;

			GLuint* textureViewsPtr = glTextureViews;
			ENQUEUE_DEFERRED_RENDER_COMMAND(
				[textureViewsPtr](RenderCommandList& cmdList) {
					gRenderDevice->deleteTextures(6, textureViewsPtr);
				}
			);
			::memset(glTextureViews, 0, 6 * sizeof(GLuint));
		}
	}

	RenderTargetView* RenderTargetCube::getRenderTargetView(uint32 faceIndex) const {
		CHECK(0 <= faceIndex && faceIndex < 6);
		return renderTargetViews[faceIndex].get();
	}

	uint32 RenderTargetCube::getWidth() const {
		return texture->getCreateParams().width;
	}

	uint32 RenderTargetCube::getNumMips() const {
		return texture->getCreateParams().mipLevels;
	}

	GLuint RenderTargetCube::getGLTextureName() const {
		return texture->internal_getGLName();
	}

	bool RenderTargetCube::isTextureValid() const {
		return texture != nullptr && texture->isCreated();
	}

	bool RenderTargetCube::isColorFormat() const {
		return RENDER_TARGET_FORMAT_isColorFormat(format);
	}

	bool RenderTargetCube::isDepthFormat() const {
		return RENDER_TARGET_FORMAT_isDepthFormat(format);
	}

}