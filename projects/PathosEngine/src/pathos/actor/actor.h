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
		Actor() = default; // Use Scene::spawnActor()
	public:
		virtual ~Actor() = default;

		void destroy();

		void registerComponent(ActorComponent* component);
		void unregisterComponent(ActorComponent* component);
		//void unregisterComponents(std::vector<ActorComponent*>& components);

	protected:
		virtual void onSpawn() {}
		virtual void onDestroy() {}
		virtual void onTick(float deltaSeconds) {}

	private:
		Scene* owner = nullptr;
		bool markedForDeath = false;
		std::vector<ActorComponent*> components;

	};

}
