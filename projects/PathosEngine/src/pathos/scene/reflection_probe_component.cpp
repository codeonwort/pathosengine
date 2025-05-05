#include "reflection_probe_component.h"
#include "pathos/scene/world.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/texture.h"
#include "pathos/render/render_target.h"
#include "pathos/render/light_probe_baker.h"
#include "pathos/render/scene_render_targets.h"

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
		proxy->positionWS    = getLocation();
		proxy->captureRadius = captureRadius;
		proxy->cubemapIndex  = cubemapIndex;
		proxy->renderTarget  = radianceCubemap.get();
		proxy->specularIBL   = specularIBL.get();

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

		SceneProxyCreateParams sceneProxyParams{
			SceneProxySource::RadianceCapture,
			tempFrameNumber,
			tempCamera,
		};
		SceneProxy* sceneProxy = scene.createRenderProxy(sceneProxyParams);
		sceneProxy->overrideSceneRenderSettings(settings);

		gEngine->internal_pushSceneProxy(sceneProxy);
	}

	void ReflectionProbeComponent::bakeIBL(Texture* cubemapArray) {
		GLuint srcCubemap = radianceCubemap->getGLTextureName();
		GLuint dstCubemap = specularIBL->getGLTextureName();
		uint32 numMips = radianceCubemap->getNumMips();
		uint32 cubemapIx = cubemapIndex;
		CHECK(cubemapIx != 0xffffffff);

		if (srcCubemap == 0 || specularIBL == 0) return;

		ENQUEUE_RENDER_COMMAND(
			[srcCubemap, dstCubemap, numMips, cubemapArray, cubemapIx](RenderCommandList& cmdList) {
				LightProbeBaker::get().bakeReflectionProbe_renderThread(cmdList, srcCubemap, dstCubemap);

				GLuint size = reflectionProbeCubemapSize;
				for (int32 mip = 0; mip < (int32)pathos::reflectionProbeNumMips; ++mip) {
					cmdList.copyImageSubData(
						dstCubemap, GL_TEXTURE_CUBE_MAP, mip, 0, 0, 0,
						cubemapArray->internal_getGLName(), GL_TEXTURE_CUBE_MAP_ARRAY,
						mip, 0, 0, cubemapIx * 6, size, size, 6);
					size /= 2;
				}
			}
		);
	}

}
