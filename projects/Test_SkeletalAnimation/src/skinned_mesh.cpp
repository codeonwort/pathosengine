#include "skinned_mesh.h"

namespace pathos {

	SkinnedMesh::SkinnedMesh(MeshGeometry* G, MeshMaterial* M) : Mesh(G, M) {}

	void SkinnedMesh::addBone(Bone&& bone) {
		bones.push_back(bone);
	}

}