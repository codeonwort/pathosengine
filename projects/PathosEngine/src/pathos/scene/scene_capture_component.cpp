#include "scene_capture_component.h"
#include "scene.h"
#include "pathos/actor/world.h"
#include "pathos/actor/actor.h"
#include "pathos/render/scene_renderer.h"
#include "pathos/render/render_device.h"
#include "pathos/render/render_target.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/scene/camera.h"
#include "pathos/util/gl_debug_group.h"

namespace pathos {

	void SceneCaptureComponent::captureScene() {
		if (renderTarget == nullptr || renderTarget->isTextureValid() == false) {
			return;
		}

		SceneRenderSettings settings;
		settings.sceneWidth = renderTarget->getWidth();
		settings.sceneHeight = renderTarget->getHeight();
		settings.enablePostProcess = !captureHDR;
		settings.finalRenderTarget = renderTarget;

		Scene& scene = getOwner()->getWorld()->getScene();

		const float aspectRatio = (float)renderTarget->getWidth() / (float)renderTarget->getHeight();

		Camera tempCamera(PerspectiveLens(fovY, aspectRatio, zNear, zFar));
		tempCamera.moveToPosition(getLocation());

		Rotator componentRotation = getRotation();
		tempCamera.setYaw(componentRotation.yaw);
		tempCamera.setPitch(componentRotation.pitch);

		SceneProxy* sceneProxy = scene.createRenderProxy(SceneProxySource::SceneCapture, sceneCaptureFrameNumber, tempCamera);
		sceneProxy->overrideSceneRenderSettings(settings);

		gEngine->pushSceneProxy(sceneProxy);

		// #todo: Increment or always zero?
		sceneCaptureFrameNumber++;
	}

}
