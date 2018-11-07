#pragma once

#include "pathos/material/material_id.h"
#include <vector>

namespace pathos {

	class Mesh;
	class MeshGeometry;
	class MeshMaterial;

	struct RenderItem {
		Mesh*         mesh;
		MeshGeometry* geometry;
		MeshMaterial* material;
		RenderItem(Mesh* inMesh, MeshGeometry* inGeometry, MeshMaterial* inMaterial)
			: mesh(inMesh)
			, geometry(inGeometry)
			, material(inMaterial)
		{}
	};

	class Renderer {
		
	public:
		Renderer()                = default;
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&)      = delete;
		virtual ~Renderer()       = default;

	protected:
		std::vector<RenderItem> renderItems[(unsigned int)MATERIAL_ID::NUM_MATERIAL_IDS];

	};

}
