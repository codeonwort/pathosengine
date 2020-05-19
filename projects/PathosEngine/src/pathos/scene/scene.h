#pragma once

#include "pathos/actor/actor.h"
#include "pathos/material/material_id.h"

#include "badger/types/matrix_types.h"

#include "gl_core.h"
#include <vector>

namespace pathos {

	// Forward declaration
	class StaticMeshComponent;
	class SkyRendering;

	// Represents a 3D scene.
	class Scene {

	public:
		Scene()                        = default;
		Scene(const Scene&)            = delete;
		Scene& operator=(const Scene&) = delete;

		//////////////////////////////////////////////////////////////////////////
		// New API

		template<typename T>
		T* spawnActor() {
			static_assert(std::is_base_of<Actor, T>::value, "T should be an Actor-derived type");

			T* actor = new T;
			actors.emplace_back(actor);
			actor->isInConstructor = false;
			actor->owner = this;
			actor->fixRootComponent();
			actor->onSpawn();

			return actor;
		}

		void destroyActor(Actor* actor);

		void tick(float deltaSeconds);

		void clearRenderProxy();
		void createRenderProxy();

		//////////////////////////////////////////////////////////////////////////
		// Old API

		void transformLightProxyToViewSpace(const matrix4& viewMatrix);

	public:
		SkyRendering* sky = nullptr;
		StaticMeshComponent* godRaySource = nullptr;

		// IBL
		GLuint irradianceMap = 0;
		GLuint prefilterEnvMap = 0;
		uint32 prefilterEnvMapMipLevels = 0;

	public:
		std::vector<struct DirectionalLightProxy*> proxyList_directionalLight; // first is sun
		std::vector<struct PointLightProxy*>       proxyList_pointLight;
		std::vector<struct ShadowMeshProxy*>       proxyList_shadowMesh;
		std::vector<struct ShadowMeshProxy*>       proxyList_wireframeShadowMesh;
		std::vector<struct StaticMeshProxy*>       proxyList_staticMesh[(uint32)MATERIAL_ID::NUM_MATERIAL_IDS];

	protected:
		// #todo-actor: Wanna represent ownership, but can't use std::unique_ptr<Actor> as it can't hold subclasses of Actor.
		std::vector<Actor*> actors;          // Actors in this scene
		std::vector<Actor*> actorsToDestroy; // Actors marked for death (destroyed in next tick)

	};

}
