#include "scene.h"
#include "pathos/scene/world.h"
#include "pathos/scene/scene_component.h"
#include "pathos/scene/reflection_probe_actor.h"
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
#include "pathos/console.h"

#include <algorithm>

namespace pathos {

	static ConsoleVariable<int32> cvar_irradianceProbeAtlas_tileSize("r.irradianceProbeAtlas.tileSize", 8, "Tile size");
	static ConsoleVariable<int32> cvar_irradianceProbeAtlas_tileCountX("r.irradianceProbeAtlas.tileCountX", 64, "Tile count in x-axis");
	static ConsoleVariable<int32> cvar_irradianceProbeAtlas_tileCountY("r.irradianceProbeAtlas.tileCountY", 64, "Tile count in y-axis");

	static ConsoleVariable<int32> cvar_numReflectionProbeUpdates("r.indirectLighting.updateReflectionProbesPerFrame", 1, "Number of reflection probes to update per frame");
	static ConsoleVariable<int32> cvar_numIrradianceProbeUpdates("r.indirectLighting.updateIrradianceProbesPerFrame", 1, "Number of irradiance probes to update per frame");

	Scene::Scene() {}
	Scene::~Scene() {}

	void Scene::updateLightProbes() {
		SCOPED_CPU_COUNTER(LightProbeSceneProxy);

		const vector3 cameraPos = getWorld()->getCamera().getPosition();

		auto compareIrradianceVolumes = [&cameraPos](const IrradianceVolumeActor* A, const IrradianceVolumeActor* B) {
			return glm::distance(A->getActorLocation(), cameraPos) < glm::distance(B->getActorLocation(), cameraPos);
		};
		auto compareReflectionProbes = [](const ReflectionProbeActor* A, const ReflectionProbeActor* B) {
			if (B->bUpdateEveryFrame == false) return false;
			if (A->lastUpdateTime == B->lastUpdateTime) return A->internal_getUpdatePhase() > B->internal_getUpdatePhase();
			return A->lastUpdateTime < B->lastUpdateTime;
		};
		std::sort(irradianceVolumes.begin(), irradianceVolumes.end(), compareIrradianceVolumes);
		std::sort(reflectionProbes.begin(), reflectionProbes.end(), compareReflectionProbes);

		int32 numProbeUpdates = std::min(cvar_numReflectionProbeUpdates.getInt(), (int32)reflectionProbes.size());
		for (int32 i = 0; i < numProbeUpdates; ++i) {
			if (reflectionProbes[i]->bUpdateEveryFrame == false) break;
			reflectionProbes[i]->captureScene();
		}

		numProbeUpdates = (irradianceVolumes.size() == 0) ? 0 : std::min(cvar_numIrradianceProbeUpdates.getInt(), (int32)irradianceVolumes[0]->numProbes());
		if (numProbeUpdates > 0) {
			initializeIrradianceProbeAtlas();
			irradianceVolumes[0]->updateProbes(irradianceProbeAtlasDesc, numProbeUpdates);
		}
	}

	void Scene::initializeIrradianceProbeAtlas() {
		if (irradianceProbeAtlas == nullptr) {
			irradianceProbeAtlasDesc.tileSize = cvar_irradianceProbeAtlas_tileSize.getInt();
			irradianceProbeAtlasDesc.tileCountX = cvar_irradianceProbeAtlas_tileCountX.getInt();
			irradianceProbeAtlasDesc.tileCountY = cvar_irradianceProbeAtlas_tileCountY.getInt();

			uint32 paddedSide = (irradianceProbeAtlasDesc.tileSize + 2);
			uint32 atlasWidth = paddedSide * irradianceProbeAtlasDesc.tileCountX;
			uint32 atlasHeight = paddedSide * irradianceProbeAtlasDesc.tileCountY;

			irradianceProbeAtlas = makeUnique<RenderTarget2D>();
			irradianceProbeAtlas->respecTexture(atlasWidth, atlasHeight, pathos::IRRADIANCE_PROBE_FORMAT, "Texture_IrradianceProbeAtlas");
			irradianceProbeAtlas->immediateUpdateResource();

			depthProbeAtlas = makeUnique<RenderTarget2D>();
			depthProbeAtlas->respecTexture(atlasWidth, atlasHeight, pathos::DEPTH_PROBE_FORMAT, "Texture_DepthProbeAtlas");
			depthProbeAtlas->immediateUpdateResource();
		}
	}

	uint32 Scene::allocateIrradianceTiles(uint32 numRequiredTiles) {
		if (irradianceProbeAtlas == nullptr) {
			return IrradianceProbeAtlasDesc::INVALID_TILE_ID;
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
		if (endID < irradianceProbeAtlasDesc.totalTileCount()) {
			irradianceTileAllocs.push_back(IrradianceTileRange{ beginID, endID });
			return beginID;
		}
		
		// Failed to allocate tiles. Find out the reason.
		uint32 remainingTiles = irradianceProbeAtlasDesc.totalTileCount();
		for (const auto& allocRange : irradianceTileAllocs) {
			remainingTiles -= allocRange.end - allocRange.begin + 1;
		}
		if (remainingTiles < numRequiredTiles) {
			LOG(LogWarning, "%s: Overflow. Required: %u, remaining: %u", __FUNCTION__, numRequiredTiles, remainingTiles);
		} else {
			LOG(LogWarning, "%s: Fragmentation. Required: %u, remaining: %u but remaining tiles are not contiguous", __FUNCTION__, numRequiredTiles, remainingTiles);
		}
		return IrradianceProbeAtlasDesc::INVALID_TILE_ID;
	}

	bool Scene::freeIrradianceTiles(uint32 firstTileID, uint32 lastTileID) {
		auto it = std::find(irradianceTileAllocs.begin(), irradianceTileAllocs.end(), IrradianceTileRange{ firstTileID, lastTileID });
		if (it != irradianceTileAllocs.end()) {
			irradianceTileAllocs.erase(it);
			return true;
		}
		return false;
	}

	void Scene::getIrradianceTileTexelOffset(uint32 tileID, uint32& outX, uint32& outY) const {
		const auto& desc = irradianceProbeAtlasDesc;
		outX = 1 + (tileID % desc.tileCountX) * (2 + desc.tileSize);
		outY = 1 + (tileID / desc.tileCountX) * (2 + desc.tileSize);
	}

	void Scene::getIrradianceTileBounds(uint32 tileID, vector4& outBounds) const {
		if (irradianceProbeAtlas != nullptr) {
			uint32 x0, y0, x1, y1;
			getIrradianceTileTexelOffset(tileID, x0, y0);
			x1 = x0 + irradianceProbeAtlasDesc.tileSize;
			y1 = y0 + irradianceProbeAtlasDesc.tileSize;
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

	void Scene::registerIrradianceVolume(IrradianceVolumeActor* actor) {
		irradianceVolumes.push_back(actor);
	}
	void Scene::unregisterIrradianceVolume(IrradianceVolumeActor* actor) {
		auto it = std::find(irradianceVolumes.begin(), irradianceVolumes.end(), actor);
		irradianceVolumes.erase(it);
	}
	void Scene::registerReflectionProbe(ReflectionProbeActor* actor) {
		reflectionProbes.push_back(actor);
	}
	void Scene::unregisterReflectionProbe(ReflectionProbeActor* actor) {
		auto it = std::find(reflectionProbes.begin(), reflectionProbes.end(), actor);
		reflectionProbes.erase(it);
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
		const bool isLightProbeRendering = pathos::isLightProbeRendering(source);

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
			proxy->irradianceTileCountX = irradianceProbeAtlasDesc.tileCountX;
			proxy->irradianceTileSize = irradianceProbeAtlasDesc.tileSize;
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
		if (isLightProbeRendering == false) {
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
		proxy->irradianceVolumeBuffer = (irradianceVolumeBuffer != nullptr) ? irradianceVolumeBuffer->internal_getGLName() : 0;

		// Update reflection probe buffer.
		if (isLightProbeRendering == false) {
			const size_t numProbes = proxy->proxyList_reflectionProbe.size();
			size_t numPrevProbes = 0;
			if (reflectionProbeBuffer != nullptr) {
				numPrevProbes = reflectionProbeBuffer->getCreateParams().bufferSize / sizeof(ReflectionProbeInfo);
			}
			if (numProbes == 0) {
				if (reflectionProbeBuffer != nullptr) {
					reflectionProbeBuffer->releaseGPUResource();
					reflectionProbeBuffer = nullptr;
				}
			} else {
				// Buffer size does not fit; recreate the buffer.
				if (numPrevProbes < numProbes || numPrevProbes > numProbes * 2) {
					if (reflectionProbeBuffer != nullptr) {
						reflectionProbeBuffer->releaseGPUResource();
					}
					BufferCreateParams createParams{
						EBufferUsage::CpuWrite,
						(uint32)(numProbes * sizeof(ReflectionProbeInfo)),
						nullptr,
						"Buffer_SSBO_IrradianceVolume",
					};
					reflectionProbeBuffer = makeUnique<Buffer>(createParams);
					reflectionProbeBuffer->createGPUResource();
				}
				// Upload the data.
				// #todo-light-probe: Reupload only if changed.
				std::vector<ReflectionProbeInfo> bufferData;
				for (const ReflectionProbeProxy* probeProxy : proxy->proxyList_reflectionProbe) {
					if (probeProxy->specularIBL == nullptr) {
						continue;
					}
					ReflectionProbeInfo bufferItem{
						probeProxy->positionWS,
						probeProxy->captureRadius,
					};
					bufferData.emplace_back(bufferItem);
				}
				reflectionProbeBuffer->writeToGPU(0, bufferData.size() * sizeof(ReflectionProbeInfo), bufferData.data());
			}
		}
		proxy->reflectionProbeBuffer = (reflectionProbeBuffer != nullptr) ? reflectionProbeBuffer->internal_getGLName() : 0;

		proxy->finalize_mainThread();

		return proxy;
	}

}
