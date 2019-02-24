#pragma once

#include "tiny_obj_loader.h"
#include "pathos/loader/imageloader.h"
#include "pathos/mesh/mesh.h"

#include <stdint.h>
#include <vector>
#include <map>
#include <set>

namespace pathos {

	struct PendingTexture {
		std::string name;
	};

	struct PendingShape {
		std::map<int32_t, std::vector<GLfloat>> positions;
		std::map<int32_t, std::vector<GLfloat>> normals;
		std::map<int32_t, std::vector<GLfloat>> texcoords;
		std::map<int32_t, std::vector<GLuint>> indices;
		std::set<int32_t> materialIDs;
	};

	// Wavefront OBJ
	class OBJLoader {

	public:
		OBJLoader();
		OBJLoader(const char* objFile, const char* mtlDir);
		OBJLoader(const OBJLoader& other) = delete;
		OBJLoader(OBJLoader&& rhs) = delete;

		// May called in worker threads
		bool load(const char* objFile, const char* mtlDir);
		void unload();

		inline uint32_t numShapes() const { return static_cast<uint32_t>(pendingShapes.size()); }
		inline const string& getShapeName(uint32_t index) const { return t_shapes[index].name; }
		
		// CAUTION: should be called within main thread
		Mesh* craftMeshFrom(const string& shapeName);
		Mesh* craftMeshFrom(uint32_t shapeIndex);
		Mesh* craftMeshFromAllShapes();

		const vector<MeshMaterial*>& getMaterials() { return materials; }

	protected:
		void analyzeMaterials(const std::vector<tinyobj::material_t>& tiny_materials, std::vector<MeshMaterial*>& output);
		void reconstructShapes(const std::vector<tinyobj::shape_t>& tiny_shapes, const tinyobj::attrib_t& attrib, std::vector<PendingShape>& pendingShape);
		MeshMaterial* getMaterial(int32_t index);
		Mesh* craftMesh(uint32_t from, uint32_t to); // both inclusive

	private:
		std::string mtlDir;

		std::vector<tinyobj::shape_t> t_shapes;
		std::vector<tinyobj::material_t> t_materials;
		tinyobj::attrib_t t_attrib;

		std::vector<PendingShape> pendingShapes;
		std::vector<MeshMaterial*> materials;
		std::map<std::string, FIBITMAP*> bitmapDB;
		ColorMaterial* defaultMaterial = nullptr;

		std::vector<bool> isPendingMaterial;
		std::map<int32_t, FIBITMAP*> pendingTextureData;
		std::map<int32_t, GLuint> textureDB;

	};

}
