#include "scene.h"
#include "pathos/console.h"
#include "pathos/scene/world.h"
#include "pathos/scene/scene_component.h"
#include "pathos/scene/light_probe_component.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/render_target.h"
#include "pathos/rhi/render_device.h"
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
			irradianceProbeAtlas->respecTexture(
				atlasWidth,
				atlasHeight,
				pathos::irradianceProbeFormat,
				"Scene_IrradianceProbeAtlas");
			irradianceProbeAtlas->immediateUpdateResource();
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
		return irradianceProbeAtlas->getGLName();
	}

	SceneProxy* Scene::createRenderProxy(SceneProxySource source, uint32 frameNumber, const Camera& camera) {
		char counterName[64];
		sprintf_s(counterName, "CreateRenderProxy (%s)", pathos::getSceneProxySourceString(source));
		SCOPED_CPU_COUNTER_STRING(counterName);

		SceneProxy* proxy = new SceneProxy(source, frameNumber, camera);

		World* const world = getWorld();

		ENQUEUE_RENDER_COMMAND([world](RenderCommandList& cmdList) {
			for (auto& actor : world->actors) {
				if (!actor->markedForDeath) {
					for (ActorComponent* actorComponent : actor->components) {
						actorComponent->updateDynamicData_renderThread(cmdList);
					}
				}
			}
		});

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
		}

		proxy->skyIrradianceMap = skyIrradianceMap;
		proxy->skyPrefilterEnvMap = skyPrefilterEnvMap;
		proxy->skyPrefilterEnvMapMipLevels = skyPrefilterEnvMapMipLevels;

		if (irradianceProbeAtlas != nullptr) {
			proxy->irradianceAtlas = irradianceProbeAtlas->getGLName();
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

		proxy->finalize_mainThread();

		return proxy;
	}

}
