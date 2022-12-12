#include "light_probe_component.h"
#include "pathos/render/render_target.h"
#include "pathos/scene/world.h"

#define RADIANCE_PROBE_CUBEMAP_SIZE    256
#define RADIANCE_PROBE_FORMAT          RenderTargetFormat::RGBA16F

namespace pathos {

	// Reminder for myself: They should be here for uniquePtr + forward decl to work.
	LightProbeComponent::LightProbeComponent() {}
	LightProbeComponent::~LightProbeComponent() {}

	void LightProbeComponent::createRenderProxy(SceneProxy* scene) {
		if (scene->sceneProxySource != SceneProxySource::MainScene) {
			return;
		}

		if (probeType == ELightProbeType::Radiance) {
			RadianceProbeProxy* proxy = ALLOC_RENDER_PROXY<RadianceProbeProxy>(scene);
			proxy->positionWS = getLocation();
			proxy->captureRadius = captureRadius;
			proxy->renderTarget = renderTarget.get();
			scene->proxyList_radianceProbe.push_back(proxy);
		} else if (probeType == ELightProbeType::Irradiance) {
			IrradianceProbeProxy* proxy = ALLOC_RENDER_PROXY<IrradianceProbeProxy>(scene);
			proxy->positionWS = getLocation();
			proxy->captureRadius = captureRadius;
			proxy->renderTarget = renderTarget.get();
			scene->proxyList_irradianceProbe.push_back(proxy);
		} else {
			CHECK_NO_ENTRY();
		}
	}

	void LightProbeComponent::captureScene(uint32 faceIndex) {
		CHECK(0 <= faceIndex && faceIndex < 6);

		if (renderTarget == nullptr) {
			renderTarget = makeUnique<RenderTargetCube>();
			renderTarget->respecTexture(RADIANCE_PROBE_CUBEMAP_SIZE, RADIANCE_PROBE_FORMAT, "RadianceProbe");
		}

		const vector3 lookAtOffsets[6] = {
			vector3(+1.0f, 0.0f, 0.0f), // posX
			vector3(-1.0f, 0.0f, 0.0f), // negX
			vector3(0.0f, +1.0f, 0.0f), // posY
			vector3(0.0f, -1.0f, 0.0f), // negY
			vector3(0.0f, 0.0f, +1.0f), // posZ
			vector3(0.0f, 0.0f, -1.0f), // negZ
		};
		// #todo-light-probe: Check up vectors
		const vector3 upVectors[6] = {
			vector3(0.0f, +1.0f, 0.0f), // posX
			vector3(0.0f, +1.0f, 0.0f), // negX
			vector3(-1.0f, 0.0f, 0.0f), // posY
			vector3(+1.0f, 0.0f, 0.0f), // negY
			vector3(0.0f, +1.0f, 0.0f), // posZ
			vector3(0.0f, +1.0f, 0.0f), // negZ
		};

		if (probeType == ELightProbeType::Radiance) {
			SceneRenderSettings settings;
			settings.sceneWidth = renderTarget->getWidth();
			settings.sceneHeight = renderTarget->getWidth();
			settings.enablePostProcess = false;
			settings.finalRenderTarget = renderTarget->getRenderTargetView(faceIndex);

			Scene& scene = getOwner()->getWorld()->getScene();
			Camera tempCamera(PerspectiveLens(90.0f, 1.0f, 0.1f, captureRadius));
			tempCamera.lookAt(getLocation(), getLocation() + lookAtOffsets[faceIndex], upVectors[faceIndex]);

			const uint32 tempFrameNumber = 0;
			SceneProxy* sceneProxy = scene.createRenderProxy(
				SceneProxySource::RadianceCapture,
				tempFrameNumber,
				tempCamera);
			sceneProxy->overrideSceneRenderSettings(settings);

			gEngine->pushSceneProxy(sceneProxy);
		} else if (probeType == ELightProbeType::Irradiance) {
			//
		} else {
			CHECK_NO_ENTRY();
		}
	}

}
