#pragma once

#include <pathos/mesh/mesh.h>
#include <pathos/camera/camera.h>
#include <memory>

namespace pathos {

	class MeshDefaultRenderer {
	private:
		void renderSub(MeshGeometry*, shared_ptr<MeshMaterial>);
	public:
		void ready();
		void render(Mesh*, Camera*);
	};
}
