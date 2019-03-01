// TODO: integrate to the engine

#pragma once

#include "pathos/mesh/mesh.h"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include "skinned_mesh.h"

#include <vector>
#include <map>

#pragma comment(lib, "assimp-vc140-mt.lib")

namespace pathos {

	class DAELoader {
		
	public:
		DAELoader();
		DAELoader(
			const char* filename, const char* material_dir,
			unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality,
			bool invertWinding = false);

		virtual ~DAELoader();

		bool load(const char* filename, unsigned int flags, bool invertWinding = false);
		bool unload();

		inline Mesh* getMesh() const { return mesh; }

	protected:
		void loadNodes();
		void loadMaterials();
		void loadMeshes(bool invertWinding);
		void loadAnimations();

		inline glm::mat4 getGlmMat(aiMatrix4x4& m) {
			return glm::transpose(glm::mat4(
				m[0][0], m[0][1], m[0][2], m[0][3],
				m[1][0], m[1][1], m[1][2], m[1][3],
				m[2][0], m[2][1], m[2][2], m[2][3],
				m[3][0], m[3][1], m[3][2], m[3][3]));
		}

	private:
		const aiScene* scene = nullptr;
		Mesh* mesh = nullptr;
		Node* root = nullptr;

		std::string materialDir;
		std::map<std::string, GLuint> textureMapping;

	};

}