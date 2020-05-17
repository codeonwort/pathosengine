#include "scene.h"
#include "pathos/console.h"
#include "pathos/light/light.h"
#include "pathos/actor/scene_component.h"
#include "pathos/light/point_light_component.h"

namespace pathos {

	static ConsoleVariable<int32_t> cvar_visualize_depth("r.visualize_depth", 0, "visualize the scene depth");

	void Scene::destroyActor(Actor* actor) {
		CHECKF(actor != nullptr, "Parameter is null");
		CHECKF(actor->owner == this, "this Actor does not belong to this Scene");

		int32 ix = -1;
		for (int32 i = 0; i < actors.size(); ++i) {
			if (actors[i] == actor) {
				ix = i;
				break;
			}
		}

		if (ix == -1) {
			// #todo-actor: Should fix if reaches here
			CHECK_NO_ENTRY();
		} else {
			actor->markedForDeath = true;
			actorsToDestroy.push_back(actors[ix]);
			actors.erase(actors.begin() + ix);
		}
	}

	void Scene::tick(float deltaSeconds) {
		// Destroy actors that were marked for death
		{
			for (auto& actor : actorsToDestroy) {
				actor->onDestroy();
				delete actor;
			}
			actorsToDestroy.clear();
		}

		// Tick
		{
			for (auto& actor : actors) {
				if (!actor->markedForDeath) {
					actor->onTick(deltaSeconds);
				}
			}
		}
	}

	void Scene::clearRenderProxy() {
		proxyList_pointLight.clear();
	}

	void Scene::createRenderProxy() {
		for (auto& actor : actors) {
			if (!actor->markedForDeath) {
				for (ActorComponent* actorComponent : actor->components) {
					actorComponent->createRenderProxy(this);
				}
			}
		}
	}

	void Scene::calculateLightBuffer() {
#if OLD_POINT_LIGHT
		const uint32 numPoints = (uint32)pointLights.size();
		pointLightBuffer.resize(numPoints);
		for (uint32 i = 0u; i < numPoints; ++i) {
			pointLightBuffer[i] = pointLights[i]->getProxy();
		}
#endif

		const uint32 numDirs = (uint32)directionalLights.size();
		directionalLightBuffer.resize(numDirs);
		for (uint32 i = 0u; i < numDirs; ++i) {
			directionalLightBuffer[i] = directionalLights[i]->getProxy();
		}
	}

	void Scene::calculateLightBufferInViewSpace(const glm::mat4& viewMatrix) {
#if OLD_POINT_LIGHT
		const uint32 numPoints = (uint32)pointLights.size();
		pointLightBuffer.resize(numPoints);
		for (uint32 i = 0u; i < numPoints; ++i) {
			pointLightBuffer[i] = pointLights[i]->getProxy();
			pointLightBuffer[i].position = glm::vec3(viewMatrix * glm::vec4(pointLightBuffer[i].position, 1.0f));
		}
#else
		for (uint32 i = 0u; i < proxyList_pointLight.size(); ++i) {
			proxyList_pointLight[i]->position
				= glm::vec3(viewMatrix * glm::vec4(proxyList_pointLight[i]->position, 1.0f));
		}
#endif

		const uint32 numDirs = (uint32)directionalLights.size();
		directionalLightBuffer.resize(numDirs);
		for (uint32 i = 0u; i < numDirs; ++i) {
			directionalLightBuffer[i] = directionalLights[i]->getProxy();
			directionalLightBuffer[i].direction = glm::vec3(viewMatrix * glm::vec4(directionalLightBuffer[i].direction, 0.0f));
		}
	}

	void Scene::add(std::initializer_list<Mesh*> newMeshes) {
		for (Mesh* mesh : newMeshes) {
			meshes.push_back(mesh);
		}
	}
}
