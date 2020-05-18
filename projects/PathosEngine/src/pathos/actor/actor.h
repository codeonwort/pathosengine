#pragma once

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include <vector>

namespace pathos {

	class Scene;
	class ActorComponent;
	class SceneComponent;

	// #note-actor: Actors are owned by Scene.
	class Actor
	{
		friend class Scene;

	protected:
		Actor() { isInConstructor = true; } // Use Scene::spawnActor()
	public:
		virtual ~Actor();

		void destroy();

		void registerComponent(ActorComponent* component);
		void unregisterComponent(ActorComponent* component);

		inline SceneComponent* getRootComponent() { return rootComponent; }
		vector3 getActorLocation() const; //{ return rootComponent->getLocation(); }
		//Rotator getActorRotation() const { return rootComponent->getRotation(); } // #todo-actor
		vector3 getActorScale() const; //{ return rootComponent->getScale(); }

		void setActorLocation(const vector3& inLocation);
		//void setActorRotation(const Rotator& inRotation); // #todo-actor
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

		Scene* owner = nullptr;
		bool isInConstructor = false;
		bool markedForDeath = false;

		SceneComponent* rootComponent = nullptr; // Every actor must have a root component
		std::vector<ActorComponent*> components;

	};

}
