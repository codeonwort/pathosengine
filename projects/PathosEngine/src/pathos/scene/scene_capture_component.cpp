#include "scene_capture_component.h"
#include "world.h"
#include "actor.h"
#include "scene.h"
#include "camera.h"

#include "pathos/render/scene_renderer.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/render_target.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/rhi/gl_debug_group.h"

namespace pathos {

	void SceneCaptureComponent::captureScene() {
		if (renderTarget == nullptr || renderTarget->isTextureValid() == false) {
			return;
		}

		SceneRenderSettings settings;
		settings.sceneWidth = renderTarget->getWidth();
		settings.sceneHeight = renderTarget->getHeight();
		settings.enablePostProcess = !captureHDR;
		settings.finalRenderTarget = renderTarget->getRenderTargetView();

		Scene& scene = getOwner()->getWorld()->getScene();

		const float aspectRatio = (float)renderTarget->getWidth() / (float)renderTarget->getHeight();

		Camera tempCamera(PerspectiveLens(fovY, aspectRatio, zNear, zFar));
		tempCamera.moveToPosition(getLocation());

		Rotator componentRotation = getRotation();
		tempCamera.setYaw(componentRotation.yaw);
		tempCamera.setPitch(componentRotation.pitch);

		SceneProxyCreateParams sceneProxyParams{
			SceneProxySource::SceneCapture,
			sceneCaptureFrameNumber,
			tempCamera,
		};

		SceneProxy* sceneProxy = scene.createRenderProxy(sceneProxyParams);
		sceneProxy->overrideSceneRenderSettings(settings);

		gEngine->internal_pushSceneProxy(sceneProxy);

		// #todo-renderer: Increment or always zero?
		sceneCaptureFrameNumber++;
	}

}
