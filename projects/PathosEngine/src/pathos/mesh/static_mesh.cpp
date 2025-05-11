#include "static_mesh.h"

namespace pathos {

	StaticMesh::StaticMesh() {
	}

	StaticMesh::StaticMesh(assetPtr<MeshGeometry> geom, assetPtr<Material> mat) {
		bool bothNull = geom == nullptr && mat == nullptr;
		bool bothValid = geom != nullptr && mat != nullptr;
		
		// Only 'both null' or 'both not null' are allowed
		CHECK(bothNull || bothValid);

		if (bothValid) {
			addSection(0, geom, mat);
		}
	}

	StaticMesh::~StaticMesh() {
		lodArray.clear();
	}

	void pathos::StaticMesh::addSection(uint32 lod, assetPtr<MeshGeometry> G, assetPtr<Material> M) {
		CHECK(lod >= 0 && G != nullptr && M != nullptr);
		if ((uint32)lodArray.size() <= lod) {
			lodArray.resize(lod + 1);
		}

		lodArray[lod].geometries.push_back(G);
		lodArray[lod].materials.push_back(M);
	}

}
