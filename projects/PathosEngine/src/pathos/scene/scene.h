#pragma once

#include "pathos/actor/actor.h"
#include "pathos/light/light.h"

#include "glm/glm.hpp"
#include <vector>

namespace pathos {

	// Forward declaration
	class Mesh;
	class DirectionalLight;
	class PointLight;
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
			actor->owner = this;
			actor->onSpawn();

			return actor;
		}

		void destroyActor(Actor* actor);

		void tick(float deltaSeconds);

		//////////////////////////////////////////////////////////////////////////
		// Old API

		inline void add(Mesh* mesh) { meshes.push_back(mesh); }
		void add(std::initializer_list<Mesh*> meshes);
		
		inline void add(DirectionalLight* light) { directionalLights.push_back(light); }
		inline void add(PointLight* light)       { pointLights.push_back(light);       }

		inline uint32 numDirectionalLights() const { return (uint32)directionalLights.size(); }
		inline uint32 numPointLights()       const { return (uint32)pointLights.size();       }

		void calculateLightBuffer(); // in world space
		void calculateLightBufferInViewSpace(const glm::mat4& viewMatrix);

		inline const GLfloat* getPointLightBuffer() const { return (GLfloat*)pointLightBuffer.data(); }
		inline uint32 getPointLightBufferSize() const { return (uint32)(sizeof(PointLightProxy) * pointLightBuffer.size()); }

		inline const GLfloat* getDirectionalLightBuffer() const { return (GLfloat*)directionalLightBuffer.data(); }
		inline uint32 getDirectionalLightBufferSize() const { return (uint32)(sizeof(DirectionalLightProxy) * directionalLightBuffer.size()); }

	public:
		SkyRendering* sky = nullptr;
		Mesh* godRaySource = nullptr;
		std::vector<Mesh*> meshes;

		std::vector<DirectionalLight*> directionalLights;
		std::vector<PointLight*> pointLights;

		// IBL
		GLuint irradianceMap = 0;
		GLuint prefilterEnvMap = 0;
		uint32 prefilterEnvMapMipLevels = 0;

	protected:
		std::vector<PointLightProxy> pointLightBuffer;
		std::vector<DirectionalLightProxy> directionalLightBuffer;

		// #todo-actor: Wanna represent ownership, but can't use std::unique_ptr<Actor> as it can't hold subclasses of Actor.
		std::vector<Actor*> actors;          // Actors in this scene
		std::vector<Actor*> actorsToDestroy; // Actors marked for death (destroyed in next tick)

	};

}
