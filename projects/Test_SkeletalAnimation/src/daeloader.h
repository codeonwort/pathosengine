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
		void loadMeshes();

	private:
		const aiScene* scene = nullptr;
		std::vector<Mesh*> meshes;

	};

}