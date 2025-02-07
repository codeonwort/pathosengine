#include "mesh.h"

namespace pathos {

	StaticMesh::StaticMesh(MeshGeometry* geom, Material* mat)
		: doubleSided(false)
	{
		bool bothNull = geom == nullptr && mat == nullptr;
		bool bothValid = geom != nullptr && mat != nullptr;
			
		// Only 'both null' or 'both not null' are allowed
		CHECK(bothNull || bothValid);

		if (bothValid)
		{
			geometries.push_back(geom);
			materials.push_back(mat);
		}
	}

	StaticMesh::~StaticMesh() {
		for (auto geom : geometries) if(geom != nullptr) delete geom;
	}

	void StaticMesh::add(MeshGeometry* G, Material* M) {
		CHECK(G != nullptr && M != nullptr);

		geometries.push_back(G);
		materials.push_back(M);
	}

	const Geometries& StaticMesh::getGeometries() { return geometries; }
	const Materials& StaticMesh::getMaterials() { return materials; }

}
