#include "scene_capture_component.h"
#include "scene.h"
#include "pathos/render/render_deferred.h"
#include "pathos/render/render_device.h"
#include "pathos/camera/camera.h"

namespace pathos {

	GLuint RENDER_TARGET_FORMAT_TO_GL_FORMAT(RenderTargetFormat RTF) {
		switch (RTF) {
		case RenderTargetFormat::RGBA16F:      return GL_RGBA16F;
		case RenderTargetFormat::RGBA16I:      return GL_RGBA16I;
		case RenderTargetFormat::RGBA16UI:     return GL_RGBA16UI;
		case RenderTargetFormat::RGBA32F:      return GL_RGBA32F;
		case RenderTargetFormat::RGBA32I:      return GL_RGBA32I;
		case RenderTargetFormat::RGBA32UI:     return GL_RGBA32UI;
		case RenderTargetFormat::DEPTH16F:     return GL_DEPTH_COMPONENT16;  // #todo-scene-capture: Is this valid internalformat?
		case RenderTargetFormat::DEPTH24F:     return GL_DEPTH_COMPONENT24;  // #todo-scene-capture: Is this valid internalformat?
		case RenderTargetFormat::DEPTH32F:     return GL_DEPTH_COMPONENT32F; // #todo-scene-capture: Is this valid internalformat?
		default:
			CHECKF(0, "Missing case");
			break;
		}
		return 0;
	}

	RenderTarget2D::~RenderTarget2D() {
		destroyResource();
	}

	void RenderTarget2D::initializeResource() {
		//
	}

	void RenderTarget2D::destroyResource() {
		if (glTextureObject != 0) {
			glDeleteTextures(1, &glTextureObject);
			glTextureObject = 0;
		}
	}

	void RenderTarget2D::respecTexture(uint32 inWidth, uint32 inHeight, RenderTargetFormat inFormat) {
		const bool invalidDimension = inWidth != 0 || inHeight != 0;
		CHECKF(invalidDimension, "Invalid width or height");

		if (invalidDimension) {
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
				cmdList.createTextures(GL_TEXTURE_2D, 1, texturePtr);
				cmdList.textureStorage2D(*texturePtr, 0, glFormat, inWidth, inHeight);
			}
		);
	}

}

namespace pathos {

	void SceneCaptureComponent::captureScene() {
		if (renderTarget == nullptr) {
			return;
		}

		renderTarget->initializeResource();

		DeferredRenderer* renderer = new DeferredRenderer(renderTarget->getWidth(), renderTarget->getHeight());

		Scene* scene = getOwner()->getOwnerScene();

		const float aspectRatio = (float)renderTarget->getWidth() / (float)renderTarget->getHeight();

		// #todo-scene-capture: Let Renderer::render() take a view family parameter, not a camera.
		PerspectiveLens lens(glm::radians(fovY * 0.5f), aspectRatio, zNear, zFar);
		Camera tempCamera(&lens);

		ENQUEUE_RENDER_COMMAND(
			[renderer, scene, &tempCamera](RenderCommandList& cmdList) {
				renderer->render(cmdList, scene, &tempCamera);
				cmdList.flushAllCommands();
			}
		);

		delete renderer;

	}

}
