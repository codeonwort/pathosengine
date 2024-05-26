#include "scene.h"
#include "pathos/console.h"
#include "pathos/scene/world.h"
#include "pathos/scene/scene_component.h"
#include "pathos/scene/reflection_probe_component.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/render_target.h"
#include "pathos/render/image_based_lighting.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/texture.h"
#include "pathos/rhi/buffer.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/log.h"

namespace pathos {

	Scene::Scene() {}
	Scene::~Scene() {}

	void Scene::initializeIrradianceProbeAtlas() {
		if (irradianceProbeAtlas == nullptr) {
			irradianceTileCountX = pathos::irradianceProbeTileCountX;
			irradianceTileCountY = pathos::irradianceProbeTileCountY;
			irradianceTileTotalCount = irradianceTileCountX * irradianceTileCountY;
			irradianceTileSize = pathos::irradianceProbeTileSize;

			uint32 paddedSide = (irradianceTileSize + 2);
			uint32 atlasWidth = paddedSide * irradianceTileCountX;
			uint32 atlasHeight = paddedSide * irradianceTileCountY;

			irradianceProbeAtlas = makeUnique<RenderTarget2D>();
			irradianceProbeAtlas->respecTexture(atlasWidth, atlasHeight, pathos::irradianceProbeFormat, "Scene_IrradianceProbeAtlas");
			irradianceProbeAtlas->immediateUpdateResource();

			depthProbeAtlas = makeUnique<RenderTarget2D>();
			depthProbeAtlas->respecTexture(atlasWidth, atlasHeight, pathos::depthProbeFormat, "Scene_DepthProbeAtlas");
			depthProbeAtlas->immediateUpdateResource();
		}
		if (irradianceVolumeBuffer == nullptr) {
			//
		}
	}

	uint32 Scene::allocateIrradianceTiles(uint32 numRequiredTiles) {
		if (irradianceProbeAtlas == nullptr) {
			return IRRADIANCE_TILE_INVALID_ID;
		}
		uint32 beginID = 0, endID = numRequiredTiles - 1;
		for (const auto& allocRange : irradianceTileAllocs) {
			if (beginID <= allocRange.end && allocRange.begin <= endID) {
				beginID = allocRange.end + 1;
				endID = beginID + numRequiredTiles - 1;
			} else {
				break;
			}
		}
		if (endID < irradianceTileTotalCount) {
			irradianceTileAllocs.push_back(IrradianceTileRange{ beginID, endID });
			return beginID;
		}
		
		// Failed to allocate tiles. Find out the reason.
		uint32 remainingTiles = irradianceTileTotalCount;
		for (const auto& allocRange : irradianceTileAllocs) {
			remainingTiles -= allocRange.end - allocRange.begin + 1;
		}
		if (remainingTiles < numRequiredTiles) {
			LOG(LogWarning, "%s: Overflow. Required: %u, remaining: %u", __FUNCTION__, numRequiredTiles, remainingTiles);
		} else {
			LOG(LogWarning, "%s: Fragmentation. Required: %u, remaining: %u but remaining tiles are not contiguous", __FUNCTION__, numRequiredTiles, remainingTiles);
		}
		return IRRADIANCE_TILE_INVALID_ID;
	}

	bool Scene::freeIrradianceTiles(uint32 firstTileID, uint32 lastTileID) {
		auto it = std::find(irradianceTileAllocs.begin(), irradianceTileAllocs.end(),
			IrradianceTileRange{ firstTileID, lastTileID });
		if (it != irradianceTileAllocs.end()) {
			irradianceTileAllocs.erase(it);
			return true;
		}
		return false;
	}

	void Scene::getIrradianceTileTexelOffset(uint32 tileID, uint32& outX, uint32& outY) const {
		outX = 1 + (tileID % irradianceTileCountX) * (2 + irradianceTileSize);
		outY = 1 + (tileID / irradianceTileCountX) * (2 + irradianceTileSize);
	}

	void Scene::getIrradianceTileBounds(uint32 tileID, vector4& outBounds) const {
		if (irradianceProbeAtlas != nullptr) {
			uint32 x0, y0, x1, y1;
			getIrradianceTileTexelOffset(tileID, x0, y0);
			x1 = x0 + irradianceTileSize;
			y1 = y0 + irradianceTileSize;
			float dx = 0.5f / (float)irradianceProbeAtlas->getWidth();
			float dy = 0.5f / (float)irradianceProbeAtlas->getHeight();
			outBounds.x = +dx + x0 / (float)irradianceProbeAtlas->getWidth();
			outBounds.y = +dy + y0 / (float)irradianceProbeAtlas->getHeight();
			outBounds.z = -dx + x1 / (float)irradianceProbeAtlas->getWidth();
			outBounds.w = -dy + y1 / (float)irradianceProbeAtlas->getHeight();
		}
	}

	GLuint Scene::getIrradianceProbeAtlasTexture() const {
		return irradianceProbeAtlas->getInternalTexture()->internal_getGLName();
	}
	GLuint Scene::getDepthProbeAtlasTexture() const {
		return depthProbeAtlas->getInternalTexture()->internal_getGLName();
	}

	void Scene::invalidateSkyLighting() {
		bInvalidateSkyLighting = true;
	}

	SceneProxy* Scene::createRenderProxy(
		SceneProxySource source,
		uint32 frameNumber,
		const Camera& camera,
		Fence* fence,
		uint64 fenceValue)
	{
		char counterName[64];
		sprintf_s(counterName, "CreateRenderProxy (%s)", pathos::getSceneProxySourceString(source));
		SCOPED_CPU_COUNTER_STRING(counterName);

		World* const world = getWorld();

		SceneProxy* proxy = new SceneProxy(source, frameNumber, camera, fence, fenceValue);

		proxy->deltaSeconds = world->getLastDeltaSeconds();

		ENQUEUE_RENDER_COMMAND([world](RenderCommandList& cmdList) {
			for (auto& actor : world->actors) {
				if (!actor->markedForDeath) {
					for (ActorComponent* actorComponent : actor->components) {
						actorComponent->updateDynamicData_renderThread(cmdList);
					}
				}
			}
		});

		// #todo-scene-proxy: Dirty hack to find first directional component.
		DirectionalLightComponent* sunComponent = nullptr;
		for (auto& actor : world->actors) {
			if (!actor->markedForDeath) {
				for (ActorComponent* actorComponent : actor->components) {
					auto component = dynamic_cast<DirectionalLightComponent*>(actorComponent);
					if (component != nullptr) {
						sunComponent = component;
						proxy->internal_setSunComponent(sunComponent);
						break;
					}
				}
			}
			if (sunComponent != nullptr) {
				break;
			}
		}

		proxy->bInvalidateSkyLighting = bInvalidateSkyLighting;
		bInvalidateSkyLighting = false;

		for (auto& actor : world->actors) {
			if (!actor->markedForDeath) {
				actor->updateTransformHierarchy();
				for (ActorComponent* actorComponent : actor->components) {
					actorComponent->createRenderProxy(proxy);
				}
			}
		}

		if (godRaySource != nullptr) {
			godRaySource->createRenderProxy_internal(proxy, proxy->godRayMeshes);
			proxy->godRayLocation = godRaySource->getLocation();
			proxy->godRayColor = godRayColor;
			proxy->godRayIntensity = godRayIntensity;
		}

		if (irradianceProbeAtlas != nullptr) {
			proxy->irradianceAtlas = irradianceProbeAtlas->getInternalTexture()->internal_getGLName();
			proxy->depthProbeAtlas = depthProbeAtlas->getInternalTexture()->internal_getGLName();
			proxy->irradianceAtlasWidth = (float)irradianceProbeAtlas->getWidth();
			proxy->irradianceAtlasHeight = (float)irradianceProbeAtlas->getHeight();
			proxy->irradianceTileCountX = irradianceTileCountX;
			proxy->irradianceTileSize = irradianceTileSize;
		}
		for (auto& actor : world->actors) {
			if (!actor->markedForDeath) {
				auto vol = dynamic_cast<IrradianceVolumeActor*>(actor);
				if (vol != nullptr) {
					vol->internal_createRenderProxy(proxy);
				}
			}
		}

		// Update irradiance volume buffer.
		{
			const size_t numVolumes = proxy->proxyList_irradianceVolume.size();
			size_t numPrevVolumes = 0;
			if (irradianceVolumeBuffer != nullptr) {
				numPrevVolumes = irradianceVolumeBuffer->getCreateParams().bufferSize / sizeof(IrradianceVolumeInfo);
			}
			if (numVolumes == 0) {
				if (irradianceVolumeBuffer != nullptr) {
					irradianceVolumeBuffer->releaseGPUResource();
					irradianceVolumeBuffer = nullptr;
				}
			} else {
				// Buffer size does not fit; recreate the buffer.
				if (numPrevVolumes < numVolumes || numPrevVolumes > numVolumes * 2) {
					if (irradianceVolumeBuffer != nullptr) {
						irradianceVolumeBuffer->releaseGPUResource();
					}
					BufferCreateParams createParams{
						EBufferUsage::CpuWrite,
						(uint32)(numVolumes * sizeof(IrradianceVolumeInfo)),
						nullptr,
						"Buffer_SSBO_IrradianceVolume",
					};
					irradianceVolumeBuffer = makeUnique<Buffer>(createParams);
					irradianceVolumeBuffer->createGPUResource();
				}
				// Upload the data.
				// #todo-light-probe: Reupload only if changed.
				std::vector<IrradianceVolumeInfo> bufferData;
				for (const IrradianceVolumeProxy* volumeProxy : proxy->proxyList_irradianceVolume) {
					IrradianceVolumeInfo bufferItem{
						volumeProxy->minBounds,
						volumeProxy->irradianceTileFirstID,
						volumeProxy->maxBounds,
						volumeProxy->numProbes,
						volumeProxy->gridSize,
						volumeProxy->captureRadius,
					};
					bufferData.emplace_back(bufferItem);
				}
				irradianceVolumeBuffer->writeToGPU(0, bufferData.size() * sizeof(IrradianceVolumeInfo), bufferData.data());
			}
		}
		proxy->irradianceVolumeBuffer = irradianceVolumeBuffer.get();

		proxy->finalize_mainThread();

		return proxy;
	}

}
