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

// Events
public:
	void onStartGameWorld();
	void onOpenOptionsWidget();
	void onCloseOptionsWidget();

private:
	void loadScene();
	void createWidgets();

	actorPtr<SkyboxActor> skybox;
	actorPtr<DirectionalLightActor> sun;

	TitleWidget* titleWidget = nullptr;
	GameOptionsWidget* optionsWidget = nullptr;
};
