#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/model_transform.h"
#include "pathos/material/material.h"

#include <vector>

namespace pathos {

	// #wip: assetPtr for geometry and material
	using Geometries = std::vector<MeshGeometry*>;
	using Materials = std::vector<Material*>;

	struct StaticMeshLOD {
		Geometries geometries;
		Materials materials;

		void setMaterial(int32 index, Material* M) { materials[index] = M; }
	};

	// static mesh asset = geometries + materials
	class StaticMesh {

	public:
		// #todo-static-mesh: Move to StaticMeshComponent
		bool doubleSided    = false;
		bool renderInternal = false;

	public:
		StaticMesh(MeshGeometry* geometry = nullptr, Material* material = nullptr);
		virtual ~StaticMesh();

		void addSection(uint32 lod, MeshGeometry* geometry, Material* material);

		inline StaticMeshLOD& getLOD(uint32 lod) { return lodArray[lod]; }
		inline const StaticMeshLOD& getLOD(uint32 lod) const { return lodArray[lod]; }

	protected:
		std::vector<StaticMeshLOD> lodArray;

	};

}
