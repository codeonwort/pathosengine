#pragma once

#include "badger/types/noncopyable.h"

#include "pathos/scene/scene.h"
#include "pathos/scene/camera.h"

namespace pathos {

	class InputManager;

	class World : public Noncopyable {
		friend class Scene;
		friend class Engine;

	public:
		World();
		virtual ~World() {}

		inline InputManager* getInputManager() const { return inputManager; }

	public:
		template<typename T>
		T* spawnActor() {
			static_assert(std::is_base_of<Actor, T>::value, "T should be an Actor-derived type");

			T* actor = new T;
			actors.emplace_back(actor);
			actor->isInConstructor = false;
			actor->owner = this;
			actor->fixRootComponent();
			actor->onSpawn();

			return actor;
		}

		void destroyActor(Actor* actor);
		void destroyAllActors();

		void tick(float deltaSeconds);

		Scene& getScene() { return scene; }
		Camera& getCamera() { return camera; }

	protected:
		virtual void onInitialize() {}
		virtual void onDestroy() {}
		virtual void onTick(float deltaSeconds) {}     // Called prior to actor ticks
		virtual void onPostTick(float deltaSeconds) {} // Called after actor ticks

	private:
		void initialize(); // Called by engine
		void destroy();    // Called by engine

		inline float getLastDeltaSeconds() const { return lastDeltaSeconds; }

	protected:
		Scene scene;
		Camera camera;

		float lastDeltaSeconds = 0.0f;

		std::vector<Actor*> actors;          // Actors in this world
		std::vector<Actor*> actorsToDestroy; // Actors marked for death (destroyed in next tick)

		InputManager* inputManager;
	};

}
