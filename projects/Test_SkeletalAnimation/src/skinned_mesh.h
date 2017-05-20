#pragma once

#include "pathos/mesh/mesh.h"

#include "bone.h"

namespace pathos {

	class SkinnedMesh : public Mesh {

	public:
		SkinnedMesh(MeshGeometry* G, MeshMaterial* M);

		void addBone(Bone&& bone);

	protected:
		std::vector<Bone> bones;

	};

}