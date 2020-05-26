#include "scene_capture_component.h"
#include "scene.h"
#include "pathos/render/render_deferred.h"
#include "pathos/render/render_device.h"
#include "pathos/render/render_target.h"
#include "pathos/camera/camera.h"
#include "pathos/util/gl_debug_group.h"

namespace pathos {

	void SceneCaptureComponent::captureScene() {
		if (renderTarget == nullptr || renderTarget->isTextureValid() == false) {
			return;
		}

		DeferredRenderer* renderer = new DeferredRenderer;
		{
			SceneRenderSettings settings;
			settings.sceneWidth = renderTarget->getWidth();
			settings.sceneHeight = renderTarget->getHeight();
			settings.enablePostProcess = false; // #todo-scene-capture: Implement this
			renderer->setSceneRenderSettings(settings);
			renderer->setFinalRenderTarget(renderTarget);
		}

		Scene* scene = getOwner()->getOwnerScene();

		const float aspectRatio = (float)renderTarget->getWidth() / (float)renderTarget->getHeight();

		// #todo-scene-capture: Let Renderer::render() take a view family parameter, not a camera.
		PerspectiveLens lens(fovY, aspectRatio, zNear, zFar);
		Camera tempCamera(&lens);
		tempCamera.moveToPosition(getLocation());

		scene->createRenderProxy();

		ScopedGpuCounter::enable = false;

		ENQUEUE_RENDER_COMMAND(
			[renderer, scene, &tempCamera](RenderCommandList& cmdList) {
				SCOPED_DRAW_EVENT(SceneCapture);

				renderer->render(cmdList, scene, &tempCamera);
			}
		);

		// #todo-scene-capture: Remove flush
		FLUSH_RENDER_COMMAND();

		ENQUEUE_RENDER_COMMAND(
			[renderer, scene, &tempCamera](RenderCommandList& cmdList) {
				renderer->releaseResources(cmdList);
			}
		);

		// #todo-scene-capture: Remove flush
		FLUSH_RENDER_COMMAND();

		ScopedGpuCounter::enable = true;

		scene->clearRenderProxy();
		
		delete renderer;

	}

}
