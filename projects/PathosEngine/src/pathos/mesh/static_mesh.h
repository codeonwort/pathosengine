#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/model_transform.h"
#include "pathos/smart_pointer.h"

#include <vector>

namespace pathos {

	class MeshGeometry;
	class Material;

	using Geometries = std::vector<assetPtr<MeshGeometry>>;
	using Materials = std::vector<assetPtr<Material>>;

	struct StaticMeshLOD {
		Geometries geometries;
		Materials materials;

		void setMaterial(int32 index, assetPtr<Material> M) { materials[index] = M; }
	};

	// static mesh asset = geometries + materials
	class StaticMesh {

	public:
		// #todo-static-mesh: Move to StaticMeshComponent
		bool doubleSided    = false;
		bool renderInternal = false;

	public:
		StaticMesh();
		StaticMesh(assetPtr<MeshGeometry> geometry, assetPtr<Material> material);
		virtual ~StaticMesh();

		void addSection(uint32 lod, assetPtr<MeshGeometry> geometry, assetPtr<Material> material);

		inline StaticMeshLOD& getLOD(uint32 lod) { return lodArray[lod]; }
		inline const StaticMeshLOD& getLOD(uint32 lod) const { return lodArray[lod]; }

	protected:
		std::vector<StaticMeshLOD> lodArray;

	};

}
