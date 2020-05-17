#pragma once

#include "pathos/actor/actor.h"
#include "pathos/light/light.h"

#include "glm/glm.hpp"
#include <vector>

#define OLD_POINT_LIGHT 0 // #todo-old-point-light

namespace pathos {

	// Forward declaration
	class Mesh;
	class DirectionalLight;
#if OLD_POINT_LIGHT
	class PointLight;
#endif
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

		void clearRenderProxy();
		void createRenderProxy();

		//////////////////////////////////////////////////////////////////////////
		// Old API

		inline void add(Mesh* mesh) { meshes.push_back(mesh); }
		void add(std::initializer_list<Mesh*> meshes);
		
		inline void add(DirectionalLight* light) { directionalLights.push_back(light); }
#if OLD_POINT_LIGHT
		inline void add(PointLight* light)       { pointLights.push_back(light);       }
#endif

		inline uint32 numDirectionalLights() const { return (uint32)directionalLights.size(); }
#if OLD_POINT_LIGHT
		inline uint32 numPointLights()       const { return (uint32)pointLights.size();       }
#endif

		void calculateLightBuffer(); // in world space
		void calculateLightBufferInViewSpace(const glm::mat4& viewMatrix);

#if OLD_POINT_LIGHT
		inline const GLfloat* getPointLightBuffer() const { return (GLfloat*)pointLightBuffer.data(); }
		inline uint32 getPointLightBufferSize() const { return (uint32)(sizeof(PointLightProxy) * pointLightBuffer.size()); }
#endif

		inline const GLfloat* getDirectionalLightBuffer() const { return (GLfloat*)directionalLightBuffer.data(); }
		inline uint32 getDirectionalLightBufferSize() const { return (uint32)(sizeof(DirectionalLightProxy) * directionalLightBuffer.size()); }

	public:
		SkyRendering* sky = nullptr;
		Mesh* godRaySource = nullptr;
		std::vector<Mesh*> meshes;

		std::vector<DirectionalLight*> directionalLights;

		// IBL
		GLuint irradianceMap = 0;
		GLuint prefilterEnvMap = 0;
		uint32 prefilterEnvMapMipLevels = 0;

	public:
		std::vector<struct PointLightProxy*> proxyList_pointLight;

	protected:
#if OLD_POINT_LIGHT
		std::vector<PointLightProxy> pointLightBuffer;
#endif
		std::vector<DirectionalLightProxy> directionalLightBuffer;

		// #todo-actor: Wanna represent ownership, but can't use std::unique_ptr<Actor> as it can't hold subclasses of Actor.
		std::vector<Actor*> actors;          // Actors in this scene
		std::vector<Actor*> actorsToDestroy; // Actors marked for death (destroyed in next tick)

	// DEPRECATED
	public:
		std::vector<PointLight*> pointLights_DEPRECATED;

	};

}
