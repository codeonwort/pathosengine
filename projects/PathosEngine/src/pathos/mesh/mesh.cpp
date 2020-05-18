#include "mesh.h"

namespace pathos {

	Mesh::Mesh(MeshGeometry* geom, Material* mat)
		: doubleSided(false)
		, castsShadow(true)
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

	Mesh::~Mesh() {
		for (auto geom : geometries) if(geom != nullptr) delete geom;
	}

	void Mesh::add(MeshGeometry* G, Material* M) {
		CHECK(G != nullptr && M != nullptr);

		geometries.push_back(G);
		materials.push_back(M);
	}

	const Geometries& Mesh::getGeometries() { return geometries; }
	const Materials& Mesh::getMaterials() { return materials; }

}
