#include "world_rc1.h"
#include "player_controller.h"

void World_RC1::onInitialize()
{
	playerController = spawnActor<PlayerController>();
}

void World_RC1::onTick(float deltaSeconds)
{
	//
}
