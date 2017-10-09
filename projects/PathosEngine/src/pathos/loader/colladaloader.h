#pragma once

#define DEPRECATED

#ifndef DEPRECATED

#include "pathos/mesh/mesh.h"
#include "assimp/scene.h"
#include <vector>

namespace pathos {

	class ColladaLoader {

	public:
		ColladaLoader(const char* file);
		inline const Geometries& getGeometries() { return geometries; }

	private:
		//vector<Mesh*> meshes;
		vector<MeshGeometry*> geometries;
		void buildGeometry(aiMesh*);

	};

}

#endif