#pragma once

#include <tiny_obj_loader.h>
#include <pathos/mesh/mesh.h>
#include <vector>

using namespace std;

namespace pathos {

	class OBJLoader {
	private:
		vector<tinyobj::shape_t> t_shapes;
		vector<tinyobj::material_t> t_materials;
		vector<MeshGeometry*> geometries;
		vector<int> materialIndices;
		vector<shared_ptr<MeshMaterial>> materials;
	public:
		OBJLoader(const char* objFile, const char* mtlDir);
		Mesh* craftMesh(size_t start, size_t end, string name);
		inline size_t numGeometries() { return geometries.size(); }
	};

}