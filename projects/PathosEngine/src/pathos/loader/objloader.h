#pragma once

#include "tiny_obj_loader.h"
#include "pathos/loader/imageloader.h"
#include "pathos/mesh/mesh.h"

#include <vector>
#include <map>
#include <set>

namespace pathos {

	struct PendingTextures {
		PendingTextures() = default;
		PendingTextures(BitmapBlob* inAlbedo, BitmapBlob* inNormal, BitmapBlob* inRoughness, BitmapBlob* inMetallic)
			: albedo(inAlbedo)
			, normal(inNormal)
			, roughness(inRoughness)
			, metallic(inMetallic)
		{
		}
		BitmapBlob* albedo;
		BitmapBlob* normal;
		BitmapBlob* roughness;
		BitmapBlob* metallic;

		GLuint glAlbedo = 0;
		GLuint glNormal = 0;
		GLuint glRoughness = 0;
		GLuint glMetallic = 0;
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
		OBJLoader(const char* inObjFile, const char* inMtlDir);
		OBJLoader(const OBJLoader& other) = delete;
		OBJLoader(OBJLoader&& rhs) = delete;

		// May called in worker threads
		bool load(const char* inObjFile, const char* inMtlDir);
		void unload();

		inline const std::string& getSourceFilepath() const { return objFile; }
		inline bool isValid() const { return bIsValid; }

		inline uint32 numShapes() const { return static_cast<uint32>(pendingShapes.size()); }
		inline const std::string& getShapeName(uint32 index) const { return t_shapes[index].name; }
		
		// CAUTION: Must be called in render thread
		Mesh* craftMeshFrom(const std::string& shapeName);

		// CAUTION: Must be called in render thread
		Mesh* craftMeshFrom(uint32 shapeIndex);

		// CAUTION: Must be called in render thread
		Mesh* craftMeshFromAllShapes();

		const std::vector<Material*>& getMaterials() { return materials; }

	protected:
		void analyzeMaterials(const std::vector<tinyobj::material_t>& tiny_materials, std::vector<Material*>& output);
		void reconstructShapes(const std::vector<tinyobj::shape_t>& tiny_shapes, const tinyobj::attrib_t& attrib, std::vector<PendingShape>& pendingShape);
		Material* getMaterial(int32 index);
		Mesh* craftMesh(uint32 from, uint32 to); // both inclusive

	private:
		std::string objFile;
		std::string mtlDir;
		bool bIsValid;

		// Fallback material if there's no matching material for that within .mtl
		ColorMaterial* defaultMaterial = nullptr;

		std::vector<tinyobj::shape_t> t_shapes;
		std::vector<tinyobj::material_t> t_materials;
		tinyobj::attrib_t t_attrib;

		std::vector<PendingShape> pendingShapes;
		std::vector<Material*> materials;

		std::map<std::string, BitmapBlob*> cachedBitmapDB;
		std::map<int32, PendingTextures> pendingTextureData; // key: material index

	};

}
