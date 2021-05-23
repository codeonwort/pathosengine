#pragma once

#include "badger/math/rotator.h"
#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/assertion/assertion.h"
#include <vector>

namespace pathos {

	class Scene;
	class World;
	class ActorComponent;
	class SceneComponent;

	// #note-actor: Actors are owned by Scene.
	class Actor
	{
		friend class Scene;
		friend class World;

	protected:
		Actor() { isInConstructor = true; } // Use Scene::spawnActor()
	public:
		virtual ~Actor();

		void destroy();
		inline bool isDestroyed() const { return markedForDeath; }

		void registerComponent(ActorComponent* component);
		void unregisterComponent(ActorComponent* component);

		inline World* getWorld() const { return owner; }

		inline SceneComponent* getRootComponent() { return rootComponent; }
		vector3 getActorLocation() const;
		Rotator getActorRotation() const;
		vector3 getActorScale() const;

		void setActorLocation(const vector3& inLocation);
		void setActorLocation(float inX, float inY, float inZ);
		void setActorRotation(const Rotator& inRotation);
		void setActorScale(const vector3& inScale);
		void setActorScale(float inScale);

	protected:
		virtual void onSpawn() {}
		virtual void onDestroy() {}
		virtual void onTick(float deltaSeconds) {}

		// CAUTION: Use if and only if in constructor
		template<typename T>
		T* createDefaultComponent()
		{
			static_assert(std::is_base_of<ActorComponent, T>::value, "T should be a subclass of ActorComponent");
			CHECKF(isInConstructor, "Can't invoke outside of constructor");

			T* component = new T;
			registerComponent(component);

			return component;
		}

		void destroyComponents();

		void setAsRootComponent(SceneComponent* sceneComponent);

	private:
		void fixRootComponent();

		World* owner = nullptr;
		bool isInConstructor = false;
		bool markedForDeath = false;

		SceneComponent* rootComponent = nullptr; // Every actor must have a root component
		std::vector<ActorComponent*> components;

	};

}
