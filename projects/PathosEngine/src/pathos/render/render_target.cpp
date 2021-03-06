#include "render_target.h"
#include "render_device.h"

namespace pathos {

	GLuint RENDER_TARGET_FORMAT_TO_GL_FORMAT(RenderTargetFormat RTF) {
		switch (RTF) {
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

	RenderTarget2D::~RenderTarget2D() {
		destroyResource();
	}

	void RenderTarget2D::respecTexture(uint32 inWidth, uint32 inHeight, RenderTargetFormat inFormat) {
		const bool validDimension = inWidth != 0 && inHeight != 0;
		CHECKF(validDimension, "Invalid width or height");

		if (!validDimension) {
			return;
		}

		destroyResource();

		width = inWidth;
		height = inHeight;
		format = inFormat;

		const GLuint glFormat = RENDER_TARGET_FORMAT_TO_GL_FORMAT(format);

		GLuint* texturePtr = &glTextureObject;
		ENQUEUE_RENDER_COMMAND(
			[texturePtr, glFormat, inWidth, inHeight](RenderCommandList& cmdList) {
				gRenderDevice->createTextures(GL_TEXTURE_2D, 1, texturePtr);
				cmdList.textureStorage2D(*texturePtr, 1, glFormat, inWidth, inHeight);
				cmdList.bindTexture(GL_TEXTURE_2D, *texturePtr);
				cmdList.objectLabel(GL_TEXTURE, *texturePtr, -1, "RenderTarget2D");
			}
		);
	}

	void RenderTarget2D::immediateUpdateResource() {
		FLUSH_RENDER_COMMAND();
	}

	void RenderTarget2D::destroyResource() {
		if (glTextureObject != 0) {
			GLuint texturePtr = glTextureObject;
			ENQUEUE_RENDER_COMMAND(
				[texturePtr](RenderCommandList& cmdList) {
					gRenderDevice->deleteTextures(1, &texturePtr);
				}
			);
			glTextureObject = 0;
		}
	}

	bool RenderTarget2D::isTextureValid() const {
		return (glTextureObject != 0 && width != 0 && height != 0);
	}

	bool RenderTarget2D::isColorFormat() const {
		switch (format)
		{
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

	bool RenderTarget2D::isDepthFormat() const {
		switch (format)
		{
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
