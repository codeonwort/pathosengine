#pragma once

#include "pathos/mesh/mesh.h"
#include "pathos/mesh/envmap.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"
#include <memory>

namespace pathos {

	class MeshDefaultRenderer {
	private:
		void renderSub(MeshGeometry*, shared_ptr<MeshMaterial>);
	public:
		void ready(); // this should be called before all render() calls for each frame
		void render(pathos::Scene* scene, Camera*); // render the total scene
		void render(Mesh*, Camera*); // render an individual object
		void render(Skybox*, Camera*); // render a skybox
	};

	class NormalRenderer {
	private:
		float normalLength;
		GLuint program;
	public:
		NormalRenderer(float normalLength = 0.5);
		void render(Mesh*, Camera*);
	};

}
