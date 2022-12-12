#include "light_probe_component.h"
#include "pathos/scene/world.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/render_target.h"
#include "pathos/render/irradiance_baker.h"

#define RADIANCE_PROBE_CUBEMAP_SIZE    256
#define RADIANCE_PROBE_MAX_MIPS        5
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
			// #note: If small mips are created, IBL baker will pick black mips
			// and it will result in too-dark IBL for non-mip0 output.
			renderTarget->respecTexture(
				RADIANCE_PROBE_CUBEMAP_SIZE,
				RADIANCE_PROBE_FORMAT,
				1, // Only mip0
				"RadianceProbe_Capture");

			specularIBL = makeUnique<RenderTargetCube>();
			specularIBL->respecTexture(
				RADIANCE_PROBE_CUBEMAP_SIZE,
				RADIANCE_PROBE_FORMAT,
				RADIANCE_PROBE_MAX_MIPS,
				"RadianceProbe_IBL");
		}

		const vector3 lookAtOffsets[6] = {
			vector3(+1.0f, 0.0f, 0.0f), // posX
			vector3(-1.0f, 0.0f, 0.0f), // negX
			vector3(0.0f, +1.0f, 0.0f), // posY
			vector3(0.0f, -1.0f, 0.0f), // negY
			vector3(0.0f, 0.0f, +1.0f), // posZ
			vector3(0.0f, 0.0f, -1.0f), // negZ
		};
		// #todo-light-probe: Check up vectors.
		// Create a mirror ball and see if mirror reflection is alright.
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
			CHECK_NO_ENTRY();
		} else {
			CHECK_NO_ENTRY();
		}
	}

	void LightProbeComponent::bakeIBL() {
		if (probeType == ELightProbeType::Radiance) {
			GLuint radianceCapture = renderTarget->getGLTexture();
			GLuint textureIBL = specularIBL->getGLTexture();
			uint32 numMips = specularIBL->getNumMips();
			ENQUEUE_RENDER_COMMAND([radianceCapture, textureIBL, numMips](RenderCommandList& cmdList) {
				IrradianceBaker::bakeSpecularIBL_renderThread(
					cmdList,
					radianceCapture,
					RADIANCE_PROBE_CUBEMAP_SIZE,
					numMips,
					textureIBL);
			});
		} else {
			CHECK_NO_ENTRY();
		}
	}

}
