#pragma once

#include "badger/types/int_types.h"
#include <vector>

namespace pathos {

	class ActorComponent;

	class Actor
	{
		
	public:
		Actor() = default;
		virtual ~Actor() = default;

		void registerComponent(ActorComponent* component);
		void unregisterComponent(ActorComponent* component);
		//void unregisterComponents(std::vector<ActorComponent*>& components);

		virtual void onTick(float deltaSeconds) {}

	private:
		std::vector<ActorComponent*> components;

	};

}
