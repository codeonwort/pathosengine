#pragma once

#include "pathos/render/render_command_list.h"
#include "pathos/material/material_id.h"

#include <vector>

namespace pathos {

	class Scene;
	class Camera;
	class Mesh;
	class MeshGeometry;
	class Material;

	struct RenderItem {
		Mesh*         mesh;
		MeshGeometry* geometry;
		Material*     material;

		RenderItem(Mesh* inMesh, MeshGeometry* inGeometry, Material* inMaterial)
			: mesh(inMesh)
			, geometry(inGeometry)
			, material(inMaterial)
		{}
	};

	class Renderer {
		
	public:
		Renderer()                = default;
		virtual ~Renderer()       = default;

		Renderer(const Renderer&)            = delete;
		Renderer& operator=(const Renderer&) = delete;

		virtual void render(RenderCommandList& cmdList, Scene* scene, Camera*) = 0;

	protected:
		std::vector<RenderItem> renderItems[(unsigned int)MATERIAL_ID::NUM_MATERIAL_IDS];

	};

}
