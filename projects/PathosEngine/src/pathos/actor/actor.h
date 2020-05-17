#pragma once

#include "badger/types/int_types.h"
#include <vector>

namespace pathos {

	class Scene;
	class ActorComponent;

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
		//void unregisterComponents(std::vector<ActorComponent*>& components);

	protected:
		virtual void onSpawn() {}
		virtual void onDestroy() {}
		virtual void onTick(float deltaSeconds) {}

		// CAUTION: Use only in constructor
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

	private:
		Scene* owner = nullptr;
		bool isInConstructor = false;
		bool markedForDeath = false;
		std::vector<ActorComponent*> components;

	};

}
