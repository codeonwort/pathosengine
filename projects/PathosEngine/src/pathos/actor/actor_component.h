#pragma once

namespace pathos {

	class Actor;

	class ActorComponent
	{
		friend class Actor;

	public:
		ActorComponent() = default;
		~ActorComponent() = default;

		void unregisterFromParent();

	protected:
		virtual void onRegister() {}   // Called when registered to an owner actor
		virtual void onUnregister() {} // Called when unregistered from the owner actor

	private:
		Actor* owner = nullptr;

	};

}
