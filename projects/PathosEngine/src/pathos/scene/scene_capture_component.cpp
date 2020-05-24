#include "scene_capture_component.h"
#include "scene.h"
#include "pathos/render/render_deferred.h"
#include "pathos/render/render_device.h"
#include "pathos/render/render_target.h"
#include "pathos/camera/camera.h"

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
