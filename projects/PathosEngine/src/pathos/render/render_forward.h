#pragma once

// TODO: Refactor MeshDefaultRenderer.
// Shaders shall be maintained here by renderer, not material.

#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"

namespace pathos {

	class MeshForwardRenderer {
	public:
		void ready();
		void render(Scene* scene, Camera*); // render the total scene
	protected:
		void createShaders();
	private:
		//
	};

}