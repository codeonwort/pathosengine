#pragma once

#include "pathos/scene/world.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

class TitleWidget;
class GameOptionsWidget;
#include "pathos_forward_decl.h"

class World_RacingTitle : public World {

protected:
	// BEGIN_INTERFACE: World
	virtual void onInitialize() override;
	virtual void onDestroy() override;
	virtual void onTick(float deltaSeconds) override;
	// END_INTERFACE: World

public:
	void onStartGameWorld();
	void onOpenOptionsWidget();

private:
	void loadScene();
	void createWidgets();

	SkyboxActor* skybox = nullptr;
	DirectionalLightActor* sun = nullptr;

	TitleWidget* titleWidget = nullptr;
	GameOptionsWidget* optionsWidget = nullptr;
};
