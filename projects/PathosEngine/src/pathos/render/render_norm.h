#pragma once

#include "pathos/mesh/mesh.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"
#include "pathos/render/envmap.h"

namespace pathos {

	// mesh normal renderer for debugging
	class NormalRenderer {
	private:
		float normalLength;
		GLuint program;
		GLuint positionLocation, normalLocation;
	public:
		NormalRenderer(float normalLength = 0.5);
		void render(Mesh*, Camera*);
	};

}
