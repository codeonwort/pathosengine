#include "scene.h"
#include "pathos/console.h"
#include "pathos/scene/world.h"
#include "pathos/scene/scene_component.h"
#include "pathos/scene/light_probe_component.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/render_target.h"
#include "pathos/rhi/render_device.h"
#include "pathos/util/cpu_profiler.h"

namespace pathos {

	Scene::Scene() {}
	Scene::~Scene() {}

	void Scene::initializeIrradianceProbeAtlas() {
		if (irradianceProbeAtlas == nullptr) {
			irradianceProbeAtlas = makeUnique<RenderTarget2D>();
			irradianceTileAllocs.resize(pathos::irradianceProbeTileCountX * pathos::irradianceProbeTileCountY, false);

			uint32 paddedSide = (pathos::irradianceProbeTileSize + 2);
			uint32 atlasWidth = paddedSide * pathos::irradianceProbeTileCountX;
			uint32 atlasHeight = paddedSide * pathos::irradianceProbeTileCountY;
			irradianceProbeAtlas->respecTexture(atlasWidth, atlasHeight, pathos::irradianceProbeFormat, "Scene_IrradianceAtlas");

			irradianceProbeAtlas->immediateUpdateResource();
		}
	}

	bool Scene::allocateIrradianceTile(uint32& outTileID, uint32& outViewportX, uint32& outViewportY) {
		if (irradianceProbeAtlas == nullptr) {
			return false;
		}
		// Super naive
		for (size_t i = 0; i < irradianceTileAllocs.size(); ++i) {
			if (irradianceTileAllocs[i] == false) {
				irradianceTileAllocs[i] = true;
				outTileID = (uint32)i;
				getIrradianceTileOffset(outTileID, outViewportX, outViewportY);
				return true;
			}
		}
		return false;
	}

	bool Scene::freeIrradianceTile(uint32 tileID) {
		if (tileID < irradianceTileAllocs.size() && irradianceTileAllocs[tileID] == true) {
			irradianceTileAllocs[tileID] = false;
			return true;
		}
		return false;
	}

	void Scene::getIrradianceTileBounds(uint32 tileID, vector4& outBounds) {
		if (irradianceProbeAtlas != nullptr) {
			uint32 x0, y0, x1, y1;
			getIrradianceTileOffset(tileID, x0, y0);
			x1 = x0 + pathos::irradianceProbeTileSize;
			y1 = y0 + pathos::irradianceProbeTileSize;
			float dx = 0.5f / (float)irradianceProbeAtlas->getWidth();
			float dy = 0.5f / (float)irradianceProbeAtlas->getHeight();
			outBounds.x = +dx + x0 / (float)irradianceProbeAtlas->getWidth();
			outBounds.y = +dy + y0 / (float)irradianceProbeAtlas->getHeight();
			outBounds.z = -dx + x1 / (float)irradianceProbeAtlas->getWidth();
			outBounds.w = -dy + y1 / (float)irradianceProbeAtlas->getHeight();
		}
	}

	void Scene::getIrradianceTileOffset(uint32 tileID, uint32& outX, uint32& outY) const {
		outX = 1 + (tileID % pathos::irradianceProbeTileCountX) * (2 + pathos::irradianceProbeTileSize);
		outY = 1 + (tileID / pathos::irradianceProbeTileCountX) * (2 + pathos::irradianceProbeTileSize);
	}

	GLuint Scene::getIrradianceAtlasTexture() const {
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

		proxy->irradianceAtlas = (irradianceProbeAtlas != nullptr) ? irradianceProbeAtlas->getGLName() : 0;

		proxy->finalize_mainThread();

		return proxy;
	}

}
