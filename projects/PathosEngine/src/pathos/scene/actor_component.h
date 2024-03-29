#pragma once

#include "pathos/rhi/render_command_list.h"

namespace pathos {

	class Actor;
	class SceneProxy;

	class ActorComponent
	{
		friend class Actor;
		friend class Scene;

	public:
		ActorComponent() = default;
		virtual ~ActorComponent() = default;

		void unregisterFromParent();

		inline Actor* getOwner() const { return owner; }

		virtual bool isSceneComponent() const { return false; }

	protected:
		virtual void onRegister() {}   // Called when registered to an owner actor
		virtual void onUnregister() {} // Called when unregistered from the owner actor

		virtual void createRenderProxy(SceneProxy* scene) {}
		virtual void updateDynamicData_renderThread(RenderCommandList& cmdList) {}

	private:
		Actor* owner = nullptr;

	};

}
