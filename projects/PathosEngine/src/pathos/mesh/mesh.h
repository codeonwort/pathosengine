#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/model_transform.h"
#include "pathos/material/material.h"

#include <vector>

namespace pathos {

	using Geometries = std::vector<MeshGeometry*>;
	using Materials = std::vector<Material*>;

	// mesh asset = geometries + materials
	class StaticMesh {

	public:
		// #todo-static-mesh: Move to StaticMeshComponent
		bool doubleSided    = false;
		bool renderInternal = false;

	public:
		StaticMesh(MeshGeometry* geometry = nullptr, Material* material = nullptr);
		virtual ~StaticMesh();

		void add(MeshGeometry* geometry, Material* material);

		const Geometries& getGeometries();
		const Materials& getMaterials();

		void setMaterial(int32 index, Material* M) { materials[index] = M; }

	protected:
		Geometries geometries;
		Materials materials;

	};

}
