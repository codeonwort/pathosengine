#include "static_mesh.h"

namespace pathos {

	StaticMesh::StaticMesh(MeshGeometry* geom, Material* mat) {
		bool bothNull = geom == nullptr && mat == nullptr;
		bool bothValid = geom != nullptr && mat != nullptr;
			
		// Only 'both null' or 'both not null' are allowed
		CHECK(bothNull || bothValid);

		if (bothValid) {
			addSection(0, geom, mat);
		}
	}

	StaticMesh::~StaticMesh() {
		for (auto& lod : lodArray) {
			for (auto geom : lod.geometries) {
				if (geom != nullptr) {
					delete geom;
				}
			}
		}
	}

	void pathos::StaticMesh::addSection(uint32 lod, MeshGeometry* G, Material* M) {
		CHECK(lod >= 0 && G != nullptr && M != nullptr);
		if ((uint32)lodArray.size() <= lod) {
			lodArray.resize(lod + 1);
		}

		lodArray[lod].geometries.push_back(G);
		lodArray[lod].materials.push_back(M);
	}

}
