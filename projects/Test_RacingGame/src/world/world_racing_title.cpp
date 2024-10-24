#include "world_racing_title.h"
#include "world_racing_game.h"
#include "widget/title_widget.h"
#include "widget/game_options_widget.h"

#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/skybox_actor.h"
#include "pathos/loader/scene_loader.h"

#define TITLE_SCENE_DESC_FILE          "resources/racing_game/title_scene.json"

void World_RacingTitle::onInitialize() {
	loadScene();
	createWidgets();
}

void World_RacingTitle::onDestroy() {
	auto rootOverlay = gEngine->getOverlayRoot();

	rootOverlay->removeChild(titleWidget);
	rootOverlay->removeChild(optionsWidget);
	delete titleWidget;
	delete optionsWidget;
}

void World_RacingTitle::onTick(float deltaSeconds) {
	if (titleWidget) titleWidget->fitToScreenSize();
	if (optionsWidget) optionsWidget->fitToScreenSize();
}

void World_RacingTitle::onStartGameWorld() {
	World* gameWorld = new World_RacingGame;
	gEngine->setWorld(gameWorld);
}

void World_RacingTitle::onOpenOptionsWidget() {
	titleWidget->setWidgetEnabled(false);
	optionsWidget->setWidgetEnabled(true);
}

void World_RacingTitle::onCloseOptionsWidget() {
	titleWidget->setWidgetEnabled(true);
	optionsWidget->setWidgetEnabled(false);
}

void World_RacingTitle::loadScene() {
	ActorBinder binder;
	binder.addBinding("Skybox", &skybox);
	binder.addBinding("Sun", &sun);

	SceneLoader sceneLoader;
	sceneLoader.loadSceneDescription(this, TITLE_SCENE_DESC_FILE, binder);
}

void World_RacingTitle::createWidgets() {
	auto rootOverlay = gEngine->getOverlayRoot();

	titleWidget = new TitleWidget(this);
	optionsWidget = new GameOptionsWidget(this);

	rootOverlay->addChild(titleWidget);
	rootOverlay->addChild(optionsWidget);

	titleWidget->setWidgetEnabled(true);
	optionsWidget->setWidgetEnabled(false);
}
