#include "pch.h"
#include "CppUnitTest.h"

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace pathos;

class EmptyWorld : public World {
	
public:
	virtual void onInitialize() override {
		getCamera().lookAt(vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f), vector3(0.0f, 1.0f, 0.0f));

		DirectionalLightActor* dirLight = spawnActor<DirectionalLightActor>();
		dirLight->setDirection(glm::normalize(vector3(-0.5f, -1.0f, 1.0f)));
		dirLight->setColorAndIlluminance(vector3(1.0f, 1.0f, 1.0f), 10.0f);

		auto skyAtmosphere = spawnActor<SkyAtmosphereActor>();

		exitCounter = 0;
	}

	virtual void onTick(float deltaSeconds) {
		if (exitCounter >= 500) {
			gEngine->stop();
		} else {
			++exitCounter;
		}
	}

private:
	uint32 exitCounter = 0;

};

namespace UnitTest {

	TEST_CLASS(TestExecutable) {

	public:
		TEST_METHOD(TestLaunchThenTerminate) {
			EngineConfig config;
			config.windowWidth = 1920 / 2;
			config.windowHeight = 1080 / 2;
			config.fullscreen = false;
			config.title = "Unit Test";
			Engine::init(0, nullptr, config);

			gEngine->setWorld(new EmptyWorld);
			gEngine->start();

			Assert::IsTrue(true, L"Failed to reach here");
		}

	};

}
