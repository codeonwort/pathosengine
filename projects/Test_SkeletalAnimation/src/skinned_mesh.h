#pragma once

#include "pathos/mesh/mesh.h"

#include "bone.h"

namespace pathos {

	class SkinnedMesh : public Mesh {

	public:
		SkinnedMesh(MeshGeometry* G, MeshMaterial* M);

		void addBone(Bone&& bone);

		// TODO: switch to hardware skinning
		void setInitialPositions(std::vector<float> positions);
		void updateSoftwareSkinning();

	protected:
		std::vector<Bone> bones;

		// TODO: switch to hardware skinning
		std::vector<float> positions;

	};

}