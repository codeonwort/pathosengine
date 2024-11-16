#pragma once

#include "pathos/rhi/render_command_list.h"

#include "badger/types/enum.h"

namespace pathos {

	class Actor;
	class SceneProxy;

	class ActorComponent
	{
		friend class Actor;
		friend class Scene;

	public:
		enum class ETickPhase : uint32 {
			None          = 0,
			PrePhysics    = 1 << 0,
			PostPhysics   = 1 << 1,
			PreActorTick  = 1 << 2,
			PostActorTick = 1 << 3,
		};

		ActorComponent() = default;
		virtual ~ActorComponent() = default;

		void unregisterFromParent();

		inline Actor* getOwner() const { return owner; }

		virtual bool isSceneComponent() const { return false; }

	protected:
		virtual void onRegister() {}   // Called when registered to an owner actor
		virtual void onUnregister() {} // Called when unregistered from the owner actor

		void setTickPhases(ETickPhase phases) { tickPhases = phases; }
		virtual void onPrePhysicsTick(float deltaSeconds) {}  // Called before PhysicsScene::update().
		virtual void onPostPhysicsTick(float deltaSeconds) {} // Called after PhysicsScene::update().
		virtual void onPreActorTick(float deltaSeconds) {}    // Called before Actor::tick().
		virtual void onPostActorTick(float deltaSeconds) {}   // Called after Actor::tick().

		virtual void createRenderProxy(SceneProxy* scene) {}
		virtual void updateDynamicData_renderThread(RenderCommandList& cmdList) {}

	private:
		Actor* owner = nullptr;
		ETickPhase tickPhases = ETickPhase::None;

	};

	ENUM_CLASS_FLAGS(ActorComponent::ETickPhase);

}
