#pragma once

#include "tiny_obj_loader.h"
#include "pathos/loader/imageloader.h"
#include "pathos/mesh/mesh.h"

#include <vector>
#include <map>
#include <set>

namespace pathos {

	struct PendingTexture {
		PendingTexture() = default;
		PendingTexture(FIBITMAP* inRawData, bool inSRGB)
			: rawData(inRawData)
			, sRGB(inSRGB)
		{
		}
		FIBITMAP* rawData;
		bool sRGB;
	};

	struct PendingShape {
		std::map<int32, std::vector<GLfloat>> positions;
		std::map<int32, std::vector<GLfloat>> normals;
		std::map<int32, std::vector<GLfloat>> texcoords;
		std::map<int32, std::vector<GLuint>> indices;
		std::set<int32> materialIDs;
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

		inline uint32 numShapes() const { return static_cast<uint32>(pendingShapes.size()); }
		inline const string& getShapeName(uint32 index) const { return t_shapes[index].name; }
		
		// CAUTION: should be called within main thread
		Mesh* craftMeshFrom(const string& shapeName);
		Mesh* craftMeshFrom(uint32 shapeIndex);
		Mesh* craftMeshFromAllShapes();

		const vector<Material*>& getMaterials() { return materials; }

	protected:
		void analyzeMaterials(const std::vector<tinyobj::material_t>& tiny_materials, std::vector<Material*>& output);
		void reconstructShapes(const std::vector<tinyobj::shape_t>& tiny_shapes, const tinyobj::attrib_t& attrib, std::vector<PendingShape>& pendingShape);
		Material* getMaterial(int32 index);
		Mesh* craftMesh(uint32 from, uint32 to); // both inclusive

	private:
		std::string mtlDir;

		// Fallback material if there's no matching material for that within .mtl
		ColorMaterial* defaultMaterial = nullptr;

		std::vector<tinyobj::shape_t> t_shapes;
		std::vector<tinyobj::material_t> t_materials;
		tinyobj::attrib_t t_attrib;

		std::vector<PendingShape> pendingShapes;
		std::vector<Material*> materials;
		std::map<std::string, FIBITMAP*> bitmapDB;

		std::vector<bool> isPendingMaterial;
		std::map<int32, PendingTexture> pendingTextureData;
		std::map<int32, GLuint> textureDB;

	};

}
