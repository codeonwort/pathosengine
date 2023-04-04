#pragma once

#include "pathos/scene/actor.h"

// Spawn a sky actor in the world and it will automatically control sky lighting.
// If multiple sky actors are spawned, only one of them contributes to sky lighting.
// See SceneRenderer::renderScene() for the priority between sky types.

namespace pathos {

	// Just a marker class.
	class SkyActor : public Actor {
		//
	};

}
