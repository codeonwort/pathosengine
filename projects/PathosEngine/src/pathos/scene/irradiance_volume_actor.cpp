#include "irradiance_volume_actor.h"
#include "pathos/scene/scene_render_settings.h"
#include "pathos/scene/world.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/image_based_lighting.h"
#include "pathos/render/image_based_lighting_baker.h"
#include "pathos/util/log.h"
#include "pathos/engine.h"

namespace pathos {

	void IrradianceVolumeActor::initializeVolume(
		const vector3& inMinBounds,
		const vector3& inMaxBounds,
		const vector3ui& inProbeGrid)
	{
		if (bVolumeInitialized) {
			return;
		}
		
		CHECK(glm::all(glm::lessThan(inMinBounds, inMaxBounds)));
		CHECK(glm::all(glm::greaterThan(inProbeGrid, vector3ui(1, 1, 1))));

		minBounds = inMinBounds;
		maxBounds = inMaxBounds;
		gridSize = inProbeGrid;

#if SEPARATE_RADIANCE_CUBEMAPS
		size_t totalProbes = gridSize.x * gridSize.y * gridSize.z;
		radianceCubemaps.resize(totalProbes);
		depthCubemaps.resize(totalProbes);
#endif
		
		setActorLocation(minBounds);
		
		currentUpdateIndex = 0;
		currentUpdatePhase = 0;
		captureRadius = glm::length((maxBounds - minBounds) / vector3(gridSize));

		bVolumeInitialized = true;
	}

	void IrradianceVolumeActor::updateProbes(const IrradianceProbeAtlasDesc& atlasDesc, int32 numSteps) {
		Scene& currentScene = getWorld()->getScene();
		if (irradianceTileFirstID == IrradianceProbeAtlasDesc::INVALID_TILE_ID) {
			irradianceTileFirstID = currentScene.allocateIrradianceTiles(numProbes());
			if (irradianceTileFirstID == IrradianceProbeAtlasDesc::INVALID_TILE_ID) {
				LOG(LogWarning, "[IrradianceVolume] Failed to allocate irradiance tiles");
				return;
			}
		}

		uint32 probeIndex = currentUpdateIndex;
		uint32 tileSize = currentScene.getIrradianceProbeAtlasDesc().tileSize;
		RenderTargetCube* radianceCubemap = getRadianceCubemapForProbe(probeIndex, tileSize);
		RenderTargetCube* depthCubemap = getDepthCubemapForProbe(probeIndex, tileSize);

		for (int32 i = 0; i < numSteps; ++i) {
			if (currentUpdatePhase <= 5) {
				uint32 faceIndex = currentUpdatePhase;
				captureFace(radianceCubemap, depthCubemap, probeIndex, faceIndex);
			} else {
				bakeIrradiance(radianceCubemap, depthCubemap, probeIndex);
			}

			currentUpdatePhase += 1;
			if (currentUpdatePhase == 7) {
				currentUpdateIndex += 1;
				currentUpdatePhase = 0;
				if (currentUpdateIndex >= numProbes()) {
					currentUpdateIndex = 0;
				}
			}
		}
	}

	void IrradianceVolumeActor::internal_createRenderProxy(SceneProxy* scene) const {
		if (hasLightingData()) {
			IrradianceVolumeProxy* proxy = ALLOC_RENDER_PROXY<IrradianceVolumeProxy>(scene);
			proxy->minBounds = minBounds;
			proxy->irradianceTileFirstID = irradianceTileFirstID;
			proxy->maxBounds = maxBounds;
			proxy->numProbes = numProbes();
			proxy->gridSize = gridSize;
			proxy->captureRadius = captureRadius;

			scene->proxyList_irradianceVolume.push_back(proxy);
		}
	}

	void IrradianceVolumeActor::onSpawn() {
		getWorld()->getScene().registerIrradianceVolume(this);
	}

	void IrradianceVolumeActor::onDestroy() {
		if (irradianceTileFirstID != IrradianceProbeAtlasDesc::INVALID_TILE_ID) {
			uint32 lastID = irradianceTileFirstID + numProbes() - 1;
			bool bFreed = getWorld()->getScene().freeIrradianceTiles(irradianceTileFirstID, lastID);
			if (!bFreed) {
				LOG(LogError, "%s: Failed to free irradiance tiles", __FUNCTION__);
			}
		}
		getWorld()->getScene().unregisterIrradianceVolume(this);
	}

	vector3 IrradianceVolumeActor::getProbeLocationByIndex(uint32 probeIndex) const {
		uint32 sizeXY = gridSize.x * gridSize.y;
		uint32 gridZ = probeIndex / sizeXY;
		uint32 k = probeIndex - gridZ * sizeXY;
		uint32 gridY = k / gridSize.x;
		uint32 gridX = k % gridSize.x;
		return getProbeLocationByCoord(gridX, gridY, gridZ);
	}

	vector3 IrradianceVolumeActor::getProbeLocationByCoord(uint32 gridX, uint32 gridY, uint32 gridZ) const {
		vector3 cellSize = (maxBounds - minBounds) / vector3(gridSize - vector3ui(1, 1, 1));
		return minBounds + cellSize * vector3(gridX, gridY, gridZ);
	}

	void IrradianceVolumeActor::captureFace(RenderTargetCube* radianceCubemap, RenderTargetCube* depthCubemap, uint32 probeIndex, uint32 faceIndex) {
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
		settings.sceneWidth        = radianceCubemap->getWidth();
		settings.sceneHeight       = radianceCubemap->getWidth();
		settings.enablePostProcess = false;
		settings.finalRenderTarget = radianceCubemap->getRenderTargetView(faceIndex);
		settings.finalDepthTarget  = depthCubemap->getRenderTargetView(faceIndex);

		vector3 probePos = getProbeLocationByIndex(probeIndex);
		const float zNear = 0.01f;
		float zFar = captureRadius;

		Scene& scene = getWorld()->getScene();
		Camera tempCamera(PerspectiveLens(90.0f, 1.0f, zNear, zFar));
		tempCamera.lookAt(probePos, probePos + lookAtOffsets[faceIndex], upVectors[faceIndex]);
		if (faceIndex != 2 && faceIndex != 3) {
			tempCamera.getLens().setProjectionFlips(true, true);
		}
		const uint32 tempFrameNumber = 0;

		SceneProxy* sceneProxy = scene.createRenderProxy(
			SceneProxySource::IrradianceCapture,
			tempFrameNumber,
			tempCamera);
		sceneProxy->overrideSceneRenderSettings(settings);

		gEngine->internal_pushSceneProxy(sceneProxy);
	}

	void IrradianceVolumeActor::bakeIrradiance(RenderTargetCube* radianceCubemap, RenderTargetCube* depthCubemap, uint32 probeIndex) {
		Scene& currentScene = getWorld()->getScene();
		GLuint inputRadianceTexture = radianceCubemap->getGLTexture();
		GLuint inputDepthTexture = depthCubemap->getGLTexture();
		GLuint RT_atlas = currentScene.getIrradianceProbeAtlasTexture();
		GLuint RT_depthAtlas = currentScene.getDepthProbeAtlasTexture();

		uint32 tileID = irradianceTileFirstID + probeIndex;
		uint32 tileSize = currentScene.getIrradianceProbeAtlasDesc().tileSize;
		vector2ui viewportOffset;
		vector4 tileBounds;
		currentScene.getIrradianceTileTexelOffset(tileID, viewportOffset.x, viewportOffset.y);
		currentScene.getIrradianceTileBounds(tileID, tileBounds);

		ENQUEUE_RENDER_COMMAND(
			[inputRadianceTexture, inputDepthTexture, RT_atlas, RT_depthAtlas, viewportOffset, tileSize](RenderCommandList& cmdList) {
				IrradianceMapBakeDesc bakeDesc;
				bakeDesc.encoding = EIrradianceMapEncoding::OctahedralNormalVector;
				bakeDesc.renderTarget = RT_atlas;
				bakeDesc.depthTarget = RT_depthAtlas;
				bakeDesc.viewportSize = tileSize;
				bakeDesc.viewportOffset = viewportOffset;
				ImageBasedLightingBaker::bakeDiffuseIBL_renderThread(cmdList, inputRadianceTexture, inputDepthTexture, bakeDesc);
			}
		);
	}

	RenderTargetCube* IrradianceVolumeActor::getRadianceCubemapForProbe(uint32 probeIndex, uint32 tileSize) {
#if SEPARATE_RADIANCE_CUBEMAPS
		if (radianceCubemaps[probeIndex] == nullptr) {
			char debugName[128];
			sprintf_s(debugName, "IrradianceProbe_Capture%u", probeIndex);

			radianceCubemaps[probeIndex] = makeUnique<RenderTargetCube>();
			radianceCubemaps[probeIndex]->respecTexture(tileSize, pathos::IRRADIANCE_PROBE_FORMAT, 1, debugName);
		}
		return radianceCubemaps[probeIndex].get();
#else
		if (singleRadianceCubemap == nullptr) {
			singleRadianceCubemap = makeUnique<RenderTargetCube>();
			singleRadianceCubemap->respecTexture(tileSize, pathos::IRRADIANCE_PROBE_FORMAT, 1, "IrradianceProbe_Capture");
		}
		return singleRadianceCubemap.get();
#endif
	}

	RenderTargetCube* IrradianceVolumeActor::getDepthCubemapForProbe(uint32 probeIndex, uint32 tileSize) {
#if SEPARATE_RADIANCE_CUBEMAPS
		if (depthCubemaps[probeIndex] == nullptr) {
			char debugName[128];
			sprintf_s(debugName, "DepthProbe_Capture%u", probeIndex);

			depthCubemaps[probeIndex] = makeUnique<RenderTargetCube>();
			depthCubemaps[probeIndex]->respecTexture(tileSize, pathos::DEPTH_PROBE_FORMAT, 1, debugName);
		}
		return depthCubemaps[probeIndex].get();
#else
		if (singleDepthCubemap == nullptr) {
			singleDepthCubemap = makeUnique<RenderTargetCube>();
			singleDepthCubemap->respecTexture(tileSize, pathos::DEPTH_PROBE_FORMAT, 1, "DepthProbe_Capture");
		}
		return singleDepthCubemap.get();
#endif
	}

}
