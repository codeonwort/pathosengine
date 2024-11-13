#include "world.h"
#include "pathos/engine.h"
#include "pathos/input/input_system.h"
#include "pathos/input/input_manager.h"
#include "pathos/input/xinput_manager.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	World::World()
		: camera(PerspectiveLens(60.0f, 16.0f / 9.0f, 0.01f, 100000.0f))
	{
		scene.owner = this;
		physicsScene.initialize();
	}

	void World::destroyActor(Actor* actor) {
		CHECKF(actor != nullptr, "Parameter is null");
		CHECKF(actor && actor->owner == this, "this Actor does not belong to this Scene");

		int32 ix = -1;
		for (int32 i = 0; i < actors.size(); ++i) {
			if (actors[i] == actor) {
				ix = i;
				break;
			}
		}

		if (ix == -1) {
			// Should fix if enters here
			CHECK_NO_ENTRY();
		} else {
			actor->markedForDeath = true;
			actorsToDestroy.push_back(actors[ix]);
			actors.erase(actors.begin() + ix);
		}
	}

	void World::destroyAllActors() {
		for (size_t i = 0; i < actors.size(); ++i) {
			Actor* actor = actors[i];
			actor->markedForDeath = true;
			actorsToDestroy.push_back(actor);
		}
		actors.clear();
	}

	void World::tick(float deltaSeconds) {
		// Destroy actors that were marked for death
		for (auto& actor : actorsToDestroy) {
			actor->onDestroy();
			delete actor;
		}
		actorsToDestroy.clear();

		// Physics Tick
		physicsScene.update(deltaSeconds);

		// Game Tick
		onTick(deltaSeconds);
		for (auto& actor : actors) {
			if (!actor->markedForDeath) {
				actor->onTick(deltaSeconds);
			}
		}

		onPostTick(deltaSeconds);

		lastDeltaSeconds = deltaSeconds;
	}

	void World::initialize() {
		inputManager = gEngine->getInputSystem()->createInputManager();
		inputManager->bindXInput(XInputUserIndex::USER0);

		scene.invalidateSkyLighting();

		onInitialize();
	}

	void World::destroy() {
		for (auto& actor : actorsToDestroy) {
			actor->onDestroy();
			delete actor;
		}
		actorsToDestroy.clear();
		for (auto& actor : actors) {
			actor->onDestroy();
			delete actor;
		}
		actors.clear();

		delete inputManager;

		onDestroy();
	}

}
