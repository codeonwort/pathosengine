#pragma once

namespace pathos {

	class Actor;
	class Scene;

	class ActorComponent
	{
		friend class Actor;
		friend class Scene; // For render proxy

	public:
		ActorComponent() = default;
		~ActorComponent() = default;

		void unregisterFromParent();

	protected:
		virtual void onRegister() {}   // Called when registered to an owner actor
		virtual void onUnregister() {} // Called when unregistered from the owner actor

		virtual void createRenderProxy(Scene* scene) {}

	private:
		Actor* owner = nullptr;

	};

}
