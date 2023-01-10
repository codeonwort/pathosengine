#include "light_probe_component.h"
#include "pathos/scene/world.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/render_target.h"
#include "pathos/render/irradiance_baker.h"

namespace pathos {
	const uint32 radianceProbeCubemapSize = 256;
	const uint32 radianceProbeNumMips = 5;
	const RenderTargetFormat radianceProbeFormat = RenderTargetFormat::RGBA16F;

	const uint32 irradianceProbeTileSize = 32;
	const uint32 irradianceProbeTileCountX = 16;
	const uint32 irradianceProbeTileCountY = 16;
	const RenderTargetFormat irradianceProbeFormat = RenderTargetFormat::RGBA16F;
}

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
			proxy->renderTarget = radianceCubemap.get();
			proxy->specularIBL = specularIBL.get();
			scene->proxyList_radianceProbe.push_back(proxy);
		} else if (probeType == ELightProbeType::Irradiance) {
			IrradianceProbeProxy* proxy = ALLOC_RENDER_PROXY<IrradianceProbeProxy>(scene);
			proxy->positionWS = getLocation();
			proxy->captureRadius = captureRadius;
			proxy->irradianceTileBounds = irradianceTileBounds;
			proxy->irradianceTileID = irradianceTileID;
			proxy->renderTarget = radianceCubemap.get();
			scene->proxyList_irradianceProbe.push_back(proxy);
		} else {
			CHECK_NO_ENTRY();
		}
	}

	void LightProbeComponent::captureScene(uint32 faceIndex) {
		CHECK(0 <= faceIndex && faceIndex < 6);
		CHECK(probeType != ELightProbeType::Unknown);

		if (radianceCubemap == nullptr) {
			radianceCubemap = makeUnique<RenderTargetCube>();

			if (probeType == ELightProbeType::Radiance) {
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
			} else if (probeType == ELightProbeType::Irradiance) {
				radianceCubemap->respecTexture(
					irradianceProbeTileSize,
					irradianceProbeFormat,
					1,
					"IrradianceProbe_Capture");
			}
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
			(probeType == ELightProbeType::Radiance) ? SceneProxySource::RadianceCapture : SceneProxySource::IrradianceCapture,
			tempFrameNumber,
			tempCamera);
		sceneProxy->overrideSceneRenderSettings(settings);

		gEngine->pushSceneProxy(sceneProxy);
	}

	void LightProbeComponent::bakeIBL() {
		if (probeType == ELightProbeType::Radiance) {
			GLuint radianceCapture = radianceCubemap->getGLTexture();
			GLuint textureIBL = specularIBL->getGLTexture();
			uint32 numMips = specularIBL->getNumMips();
			ENQUEUE_RENDER_COMMAND([radianceCapture, textureIBL, numMips](RenderCommandList& cmdList) {
				IrradianceBaker::bakeSpecularIBL_renderThread(
					cmdList,
					radianceCapture,
					radianceProbeCubemapSize,
					numMips,
					textureIBL);
			});
		} else {
			Scene& currentScene = getOwner()->getWorld()->getScene();
			GLuint radianceCapture = radianceCubemap->getGLTexture();
			GLuint RT_atlas = currentScene.getIrradianceAtlasTexture();
			bool bValidTile = (irradianceTileID != 0xffffffff);
			if (!bValidTile) {
				bValidTile = currentScene.allocateIrradianceTile(irradianceTileID, irradianceRenderOffset.x, irradianceRenderOffset.y);
			}

			if (bValidTile) {
				currentScene.getIrradianceTileBounds(irradianceTileID, irradianceTileBounds);

				vector2ui viewportOffset = irradianceRenderOffset;
				ENQUEUE_RENDER_COMMAND([radianceCapture, RT_atlas, viewportOffset](RenderCommandList& cmdList) {
					IrradianceMapBakeDesc bakeDesc;
					bakeDesc.encoding = EIrradianceMapEncoding::OctahedralNormalVector;
					bakeDesc.renderTarget = RT_atlas;
					bakeDesc.viewportSize = irradianceProbeTileSize;
					bakeDesc.viewportOffset = viewportOffset;
					IrradianceBaker::bakeDiffuseIBL_renderThread(cmdList, radianceCapture, bakeDesc);
				});
			}
		}
	}

}
