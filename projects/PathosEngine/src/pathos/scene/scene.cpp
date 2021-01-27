#include "scene.h"
#include "pathos/console.h"
#include "pathos/actor/world.h"
#include "pathos/actor/scene_component.h"
#include "pathos/light/point_light_component.h"
#include "pathos/light/directional_light_component.h"

namespace pathos {

	void Scene::clearRenderProxy() {
		proxyList_directionalLight.clear();
		proxyList_pointLight.clear();
		proxyList_shadowMesh.clear();
		proxyList_wireframeShadowMesh.clear();
		for (uint32 i = 0; i < (uint32)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
			proxyList_staticMesh[i].clear();
		}
	}

	void Scene::createRenderProxy() {
		for (auto& actor : getWorld()->actors) {
			if (!actor->markedForDeath) {
				for (ActorComponent* actorComponent : actor->components) {
					actorComponent->createRenderProxy(this);
				}
			}
		}
	}

	void Scene::createViewDependentRenderProxy(const matrix4& viewMatrix) {
		for (uint32 i = 0u; i < proxyList_pointLight.size(); ++i) {
			proxyList_pointLight[i]->viewPosition = vector3(viewMatrix * vector4(proxyList_pointLight[i]->worldPosition, 1.0f));
		}

		for (uint32 i = 0u; i < proxyList_directionalLight.size(); ++i) {
			proxyList_directionalLight[i]->direction = vector3(viewMatrix * vector4(proxyList_directionalLight[i]->direction, 0.0f));
		}
	}

}
