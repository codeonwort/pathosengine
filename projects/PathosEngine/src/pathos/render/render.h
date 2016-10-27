#pragma once

#include <pathos/mesh/mesh.h>
#include <pathos/mesh/envmap.h>
#include <pathos/camera/camera.h>
#include <memory>

namespace pathos {

	class MeshDefaultRenderer {
	private:
		void renderSub(MeshGeometry*, shared_ptr<MeshMaterial>);
	public:
		void ready();
		void render(Mesh*, Camera*);
		void render(Skybox*, Camera*);
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
