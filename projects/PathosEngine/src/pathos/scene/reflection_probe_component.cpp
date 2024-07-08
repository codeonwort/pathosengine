#include "reflection_probe_component.h"
#include "pathos/scene/world.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/render_target.h"
#include "pathos/render/image_based_lighting_baker.h"

namespace pathos {
	const uint32 reflectionProbeCubemapSize = 128; // #todo-light-probe: Cubemap size is forced to 128.
	const uint32 reflectionProbeNumMips = 7;
	const RenderTargetFormat reflectionProbeFormat = RenderTargetFormat::RGBA16F;
}

namespace pathos {

	// Reminder for myself: They should be here for uniquePtr + forward decl to work.
	ReflectionProbeComponent::ReflectionProbeComponent() {}
	ReflectionProbeComponent::~ReflectionProbeComponent() {}

	void ReflectionProbeComponent::createRenderProxy(SceneProxy* scene) {
		if (scene->sceneProxySource != SceneProxySource::MainScene) {
			return;
		}

		ReflectionProbeProxy* proxy = ALLOC_RENDER_PROXY<ReflectionProbeProxy>(scene);
		proxy->positionWS = getLocation();
		proxy->captureRadius = captureRadius;
		proxy->renderTarget = radianceCubemap.get();
		proxy->specularIBL = specularIBL.get();

		scene->proxyList_reflectionProbe.push_back(proxy);
	}

	void ReflectionProbeComponent::captureScene(uint32 faceIndex) {
		CHECK(0 <= faceIndex && faceIndex < 6);

		if (radianceCubemap == nullptr) {
			radianceCubemap = makeUnique<RenderTargetCube>();

			// #note: If small mips are created, IBL baker will pick black mips
			// and it will result in too-dark IBL for non-mip0 output.
			radianceCubemap->respecTexture(
				reflectionProbeCubemapSize,
				reflectionProbeFormat,
				reflectionProbeNumMips, //1, // Only mip0
				"ReflectionProbe_Capture");

			specularIBL = makeUnique<RenderTargetCube>();
			specularIBL->respecTexture(
				reflectionProbeCubemapSize,
				reflectionProbeFormat,
				reflectionProbeNumMips,
				"ReflectionProbe_IBL");
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

		gEngine->internal_pushSceneProxy(sceneProxy);
	}

	void ReflectionProbeComponent::bakeIBL() {
		GLuint srcCubemap = radianceCubemap->getGLTexture();
		GLuint dstCubemap = specularIBL->getGLTexture();
		if (srcCubemap == 0 || specularIBL == 0) return;
		uint32 numMips = radianceCubemap->getNumMips();
		ENQUEUE_RENDER_COMMAND(
			[srcCubemap, dstCubemap, numMips](RenderCommandList& cmdList) {
				ImageBasedLightingBaker::bakeReflectionProbe_renderThread(cmdList, srcCubemap, dstCubemap);
			}
		);
	}

}
