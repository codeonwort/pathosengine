#include "light_probe_component.h"
#include "pathos/scene/world.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/render_target.h"
#include "pathos/render/irradiance_baker.h"

namespace pathos {
	const uint32 radianceProbeCubemapSize = 256;
	const uint32 radianceProbeNumMips = 5;
	const RenderTargetFormat radianceProbeFormat = RenderTargetFormat::RGBA16F;
}

namespace pathos {

	// Reminder for myself: They should be here for uniquePtr + forward decl to work.
	LightProbeComponent::LightProbeComponent() {}
	LightProbeComponent::~LightProbeComponent() {}

	void LightProbeComponent::createRenderProxy(SceneProxy* scene) {
		if (scene->sceneProxySource != SceneProxySource::MainScene) {
			return;
		}

		RadianceProbeProxy* proxy = ALLOC_RENDER_PROXY<RadianceProbeProxy>(scene);
		proxy->positionWS = getLocation();
		proxy->captureRadius = captureRadius;
		proxy->renderTarget = radianceCubemap.get();
		proxy->specularIBL = specularIBL.get();

		scene->proxyList_radianceProbe.push_back(proxy);
	}

	void LightProbeComponent::captureScene(uint32 faceIndex) {
		CHECK(0 <= faceIndex && faceIndex < 6);

		if (radianceCubemap == nullptr) {
			radianceCubemap = makeUnique<RenderTargetCube>();

			// #note: If small mips are created, IBL baker will pick black mips
			// and it will result in too-dark IBL for non-mip0 output.
			radianceCubemap->respecTexture(
				radianceProbeCubemapSize,
				radianceProbeFormat,
				1, // Only mip0
				"RadianceProbe_Capture");

			specularIBL = makeUnique<RenderTargetCube>();
			specularIBL->respecTexture(
				radianceProbeCubemapSize,
				radianceProbeFormat,
				radianceProbeNumMips,
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
		const vector3 upVectors[6] = {
			vector3(0.0f, -1.0f, 0.0f), // posX
			vector3(0.0f, -1.0f, 0.0f), // negX
			vector3(+1.0f, 0.0f, 0.0f), // posY
			vector3(-1.0f, 0.0f, 0.0f), // negY
			vector3(0.0f, -1.0f, 0.0f), // posZ
			vector3(0.0f, -1.0f, 0.0f), // negZ
		};

		SceneRenderSettings settings;
		settings.sceneWidth = radianceCubemap->getWidth();
		settings.sceneHeight = radianceCubemap->getWidth();
		settings.enablePostProcess = false;
		settings.finalRenderTarget = radianceCubemap->getRenderTargetView(faceIndex);

		Scene& scene = getOwner()->getWorld()->getScene();
		Camera tempCamera(PerspectiveLens(90.0f, 1.0f, 0.1f, captureRadius));
		tempCamera.lookAt(getLocation(), getLocation() + lookAtOffsets[faceIndex], upVectors[faceIndex]);
		if (faceIndex != 2 && faceIndex != 3) {
			tempCamera.getLens().setProjectionFlips(true, true);
		}
		const uint32 tempFrameNumber = 0;

		SceneProxy* sceneProxy = scene.createRenderProxy(
			SceneProxySource::RadianceCapture,
			tempFrameNumber,
			tempCamera);
		sceneProxy->overrideSceneRenderSettings(settings);

		gEngine->pushSceneProxy(sceneProxy);
	}

	void LightProbeComponent::bakeIBL() {
		GLuint radianceCapture = radianceCubemap->getGLTexture();
		GLuint textureIBL = specularIBL->getGLTexture();
		uint32 numMips = specularIBL->getNumMips();
		ENQUEUE_RENDER_COMMAND(
			[radianceCapture, textureIBL, numMips](RenderCommandList& cmdList) {
				IrradianceBaker::bakeSpecularIBL_renderThread(
					cmdList,
					radianceCapture,
					radianceProbeCubemapSize,
					numMips,
					textureIBL);
			}
		);
	}

}
