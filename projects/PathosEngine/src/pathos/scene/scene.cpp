#include "scene.h"
#include "pathos/console.h"
#include "pathos/scene/world.h"
#include "pathos/scene/scene_component.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/rhi/render_device.h"

namespace pathos {

	SceneProxy* Scene::createRenderProxy(SceneProxySource source, uint32 frameNumber, const Camera& camera) {
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

		proxy->irradianceMap = irradianceMap;
		proxy->prefilterEnvMap = prefilterEnvMap;
		proxy->prefilterEnvMapMipLevels = prefilterEnvMapMipLevels;

		proxy->finalize_mainThread();

		return proxy;
	}

}
