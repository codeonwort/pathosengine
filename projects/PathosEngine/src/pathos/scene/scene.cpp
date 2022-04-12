#include "scene.h"
#include "pathos/console.h"
#include "pathos/actor/world.h"
#include "pathos/actor/scene_component.h"
#include "pathos/light/point_light_component.h"
#include "pathos/light/directional_light_component.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/render/scene_proxy.h"

namespace pathos {

	SceneProxy* Scene::createRenderProxy(uint32 frameNumber, const Camera& camera) {
		SceneProxy* proxy = new SceneProxy(frameNumber, camera);

		for (auto& actor : getWorld()->actors) {
			if (!actor->markedForDeath) {
				for (ActorComponent* actorComponent : actor->components) {
					actorComponent->createRenderProxy(proxy);
				}
			}
		}

		if (godRaySource != nullptr) {
			godRaySource->createRenderProxy_internal(proxy, proxy->godRayMeshes);
			proxy->godRayLocation = godRaySource->getLocation();
		}

		proxy->irradianceMap = irradianceMap;
		proxy->prefilterEnvMap = prefilterEnvMap;
		proxy->prefilterEnvMapMipLevels = prefilterEnvMapMipLevels;

		return proxy;
	}

	void Scene::updateDynamicData_renderThread(RenderCommandList& cmdList)
	{
		for (auto& actor : getWorld()->actors) {
			if (!actor->markedForDeath) {
				for (ActorComponent* actorComponent : actor->components) {
					actorComponent->updateDynamicData_renderThread(cmdList);
				}
			}
		}
	}

}
