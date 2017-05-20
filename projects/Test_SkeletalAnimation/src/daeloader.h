// TODO: integrate to the engine

#pragma once

#include "pathos/mesh/mesh.h"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include <vector>

namespace pathos {

	class DAELoader {
		
	public:
		DAELoader();
		DAELoader(const char* filename, unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality);
		virtual ~DAELoader();

		bool load(const char* filename, unsigned int flags);
		bool unload();

		inline const std::vector<Mesh*>& getMeshes() { return meshes; }

	protected:
		void loadNodes();
		void loadAnimations();
		void loadMaterials();
		void loadMeshes();

		inline glm::mat4 getGlmMat(aiMatrix4x4& m) {
			return glm::transpose(glm::mat4(
				m[0][0], m[0][1], m[0][2], m[0][3],
				m[1][0], m[1][1], m[1][2], m[1][3],
				m[2][0], m[2][1], m[2][2], m[2][3],
				m[3][0], m[3][1], m[3][2], m[3][3]));
		}

	private:
		const aiScene* scene = nullptr;
		std::vector<Mesh*> meshes;

	};

}