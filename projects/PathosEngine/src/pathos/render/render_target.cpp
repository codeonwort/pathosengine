#include "render_target.h"
#include "pathos/rhi/render_device.h"

namespace pathos {

	GLenum RENDER_TARGET_FORMAT_TO_GL_FORMAT(RenderTargetFormat RTF) {
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

	bool RENDER_TARGET_FORMAT_isColorFormat(RenderTargetFormat format) {
		switch (format) {
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
			return renderTarget2D->getGLName();
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

		width = inWidth;
		height = inHeight;
		format = inFormat;

		const GLenum glFormat = RENDER_TARGET_FORMAT_TO_GL_FORMAT(format);

		GLuint* texturePtr = &glTextureObject;
		std::string debugName = inDebugName ? inDebugName : std::string();
		ENQUEUE_RENDER_COMMAND(
			[texturePtr, glFormat, inWidth, inHeight, debugName](RenderCommandList& cmdList) {
				gRenderDevice->createTextures(GL_TEXTURE_2D, 1, texturePtr);
				cmdList.textureStorage2D(*texturePtr, 1, glFormat, inWidth, inHeight);
				if (debugName.size() > 0) {
					cmdList.objectLabel(GL_TEXTURE, *texturePtr, -1, debugName.c_str());
				} else {
					cmdList.objectLabel(GL_TEXTURE, *texturePtr, -1, "RenderTarget2D_noname");
				}
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

	RenderTargetView* RenderTarget2D::getRenderTargetView() const {
		return renderTargetView.get();
	}

	bool RenderTarget2D::isTextureValid() const {
		return (glTextureObject != 0 && width != 0 && height != 0);
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

		width = inWidth;
		format = inFormat;

		const GLenum glFormat = RENDER_TARGET_FORMAT_TO_GL_FORMAT(format);
		
		uint32 fullMips = (uint32)(floor(log2(width)) + 1);
		numMips = (inNumMips == 0) ? fullMips : std::min(inNumMips, fullMips);

		GLuint* texturePtr = &glTextureObject;
		GLuint* textureViewsPtr = glTextureViews;
		std::string debugName = inDebugName ? inDebugName : std::string();
		ENQUEUE_RENDER_COMMAND(
			[texturePtr, textureViewsPtr, glFormat, inWidth, numMips = this->numMips, debugName](RenderCommandList& cmdList) {
				// Cubemap texture
				gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, texturePtr);
				if (debugName.size() > 0) {
					gRenderDevice->objectLabel(GL_TEXTURE, *texturePtr, -1, debugName.c_str());
				}

				cmdList.textureStorage2D(*texturePtr, numMips, glFormat, inWidth, inWidth);

				// Texture views
				gRenderDevice->genTextures(6, textureViewsPtr);
				for (uint32 i = 0; i < 6; ++i) {
					GLuint view = textureViewsPtr[i];
					cmdList.textureView(
						view, GL_TEXTURE_2D, *texturePtr, glFormat,
						0, 1, // LOD0
						i, 1  // layer[i]
					);
					if (debugName.size() > 0) {
						std::string viewName = debugName + "_view" + std::to_string(i);
						cmdList.objectLabel(GL_TEXTURE, view, -1, viewName.c_str());
					}
				}

				// #todo-light-probe: Fill initial data with zero?
				//for (int32 i = 0; i < 6; i++) {
				//	uint8* data = nullptr;
				//	cmdList.textureSubImage3D(*texturePtr, 0,
				//		0, 0, i,
				//		inWidth, inWidth, 1,
				//		glFormat, GL_UNSIGNED_BYTE, data);
				//}
			}
		);
	}

	void RenderTargetCube::immediateUpdateResources() {
		FLUSH_RENDER_COMMAND();
	}

	void RenderTargetCube::destroyResources()
	{
		if (glTextureObject != 0) {
			GLuint texturePtr = glTextureObject;
			GLuint* textureViewsPtr = glTextureViews;
			ENQUEUE_RENDER_COMMAND(
				[texturePtr, textureViewsPtr](RenderCommandList& cmdList) {
					gRenderDevice->deleteTextures(1, &texturePtr);
					gRenderDevice->deleteTextures(6, textureViewsPtr);
				}
			);
			glTextureObject = 0;
			::memset(glTextureViews, 0, 6 * sizeof(GLuint));
		}
	}

	RenderTargetView* RenderTargetCube::getRenderTargetView(uint32 faceIndex) const {
		CHECK(0 <= faceIndex && faceIndex < 6);
		return renderTargetViews[faceIndex].get();
	}

	bool RenderTargetCube::isTextureValid() const {
		return (glTextureObject != 0 && width != 0);
	}

	bool RenderTargetCube::isColorFormat() const {
		return RENDER_TARGET_FORMAT_isColorFormat(format);
	}

	bool RenderTargetCube::isDepthFormat() const {
		return RENDER_TARGET_FORMAT_isDepthFormat(format);
	}

}