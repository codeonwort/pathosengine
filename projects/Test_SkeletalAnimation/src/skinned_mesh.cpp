#include "skinned_mesh.h"

namespace pathos {

	SkinnedMesh::SkinnedMesh(MeshGeometry* G, MeshMaterial* M) : Mesh(G, M) {}

	void SkinnedMesh::addBone(Bone&& bone) {
		bones.push_back(bone);
	}

	// TODO: switch to hardware skinning
	void SkinnedMesh::setInitialPositions(std::vector<float> positions) {
		this->positions = positions;
	}
	void SkinnedMesh::updateSoftwareSkinning() {
		const auto G = geometries[0];
		std::vector<float> pos(positions.size(), 0.0f);

		for (auto i = 0; i < bones.size(); ++i) {
			const auto& bone = bones[i];
			for (auto j = 0; j < bone.weights.size(); ++j) {
				auto p = 3 * bone.vertexIDs[j];
				auto v = bone.weights[j] * bone.offset * glm::vec4(positions[p], positions[p + 1], positions[p + 2], 1.0f);
				pos[p] += v.x;
				pos[p + 1] += v.y;
				pos[p + 2] += v.z;
			}
		}

		G->updateVertexData(&pos[0], pos.size());
	}

}