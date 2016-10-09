#pragma once

#include <tiny_obj_loader.h>
#include <pathos/mesh/mesh.h>
#include <assimp/scene.h>
#include <vector>

using namespace std;

namespace pathos {

	class OBJLoader {
	private:
		vector<tinyobj::shape_t> t_shapes;
		vector<tinyobj::material_t> t_materials;
		vector<MeshGeometry*> geometries;
		vector<int> materialIndices; // material index for each geometry
		vector<shared_ptr<MeshMaterial>> materials;
	public:
		OBJLoader(const char* objFile, const char* mtlDir);
		Mesh* craftMesh(size_t start, size_t end, string name);
		const vector<shared_ptr<MeshMaterial>>& getMaterials() { return materials; }
		const vector<MeshGeometry*>& getGeometries() { return geometries; }
		inline size_t numGeometries() { return geometries.size(); }
	};

	class ColladaLoader {
	private:
		//vector<Mesh*> meshes;
		vector<MeshGeometry*> geometries;
		void buildGeometry(aiMesh*);
	public:
		ColladaLoader(const char* file);
		inline const Geometries& getGeometries() { return geometries; }
	};

}