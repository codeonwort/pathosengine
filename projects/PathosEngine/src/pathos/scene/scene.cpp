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

	static ConsoleVariable<int32> cvar_numReflectionProbeUpdates("r.lightProbe.updateSpecularPerFrame", 1, "Number of reflection probes to update per frame");
	static ConsoleVariable<int32> cvar_numIrradianceProbeUpdates("r.lightProbe.updateDiffusePerFrame", 1, "Number of irradiance probes to update per frame");

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
			lightProbeScene.createIrradianceProbeAtlas();
			irradianceVolumes[0]->updateProbes(lightProbeScene.getIrradianceProbeAtlasDesc(), numProbeUpdates);
		}
	}

	void Scene::initializeIrradianceProbeAtlasDesc(const IrradianceProbeAtlasDesc& desc) {
		lightProbeScene.initializeIrradianceProbeAtlasDesc(desc);
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

	SceneProxy* Scene::createRenderProxy(const SceneProxyCreateParams& createParams) {
		SceneProxySource source = createParams.proxySource;
		uint32 frameNumber      = createParams.frameNumber;
		const Camera& camera    = createParams.camera;
		Fence* fence            = createParams.fence;
		uint64 fenceValue       = createParams.fenceValue;

		char counterName[64];
		sprintf_s(counterName, "CreateRenderProxy (%s)", pathos::getSceneProxySourceString(source));
		SCOPED_CPU_COUNTER_STRING(counterName);

		World* const world = getWorld();
		const bool isLightProbeRendering = pathos::isLightProbeRendering(source);

		SceneProxy* proxy = new SceneProxy(createParams);

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

		for (auto& actor : world->actors) {
			if (!actor->markedForDeath) {
				auto vol = dynamic_cast<IrradianceVolumeActor*>(actor);
				if (vol != nullptr) {
					vol->internal_createRenderProxy(proxy);
				}
			}
		}
		lightProbeScene.createSceneProxy(proxy, isLightProbeRendering);

		proxy->finalize_mainThread();

		return proxy;
	}

}
