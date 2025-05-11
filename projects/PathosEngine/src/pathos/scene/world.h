#pragma once

#include "badger/types/noncopyable.h"
#include "badger/physics/physics_scene.h"

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
		actorPtr<T> spawnActor() {
			static_assert(std::is_base_of<Actor, T>::value, "T should be an Actor-derived type");

			// #todo-memory: Can't call makeShared<Actor>() because Actor has protected constructor :(
			T* actorRaw = new T;
			actorPtr<T> actor(actorRaw);

			actors.push_back(actor);

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
		badger::physics::PhysicsScene& getPhysicsScene() { return physicsScene; }

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

		badger::physics::PhysicsScene physicsScene;

		float lastDeltaSeconds = 0.0f;

		std::vector<actorPtr<Actor>> actors;          // Actors in this world
		std::vector<actorPtr<Actor>> actorsToDestroy; // Actors marked for death (destroyed in next tick)

		InputManager* inputManager = nullptr;
	};

}
