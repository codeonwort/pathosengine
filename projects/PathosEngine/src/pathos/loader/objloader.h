#pragma once

#include "tiny_obj_loader.h"
#include "pathos/loader/imageloader.h"
#include "pathos/mesh/mesh.h"

#include <vector>
#include <string>
#include <map>
#include <set>
#include <utility>

namespace pathos {

	class Material;

	struct PendingTextures {
		PendingTextures() = default;
		PendingTextures(BitmapBlob* inAlbedo, BitmapBlob* inNormal, BitmapBlob* inRoughness, BitmapBlob* inMetallic)
			: albedo(inAlbedo)
			, normal(inNormal)
			, roughness(inRoughness)
			, metallic(inMetallic)
		{
		}
		BitmapBlob* albedo = nullptr;
		BitmapBlob* normal = nullptr;
		BitmapBlob* roughness = nullptr;
		BitmapBlob* metallic = nullptr;

		std::string albedoFilename;
		std::string normalFilename;
		std::string roughnessFilename;
		std::string metallicFilename;

		GLuint glAlbedo = 0;
		GLuint glNormal = 0;
		GLuint glRoughness = 0;
		GLuint glMetallic = 0;
	};

	struct PendingShape {
		// Key: materialID
		std::map<int32, std::vector<GLfloat>> positions;
		std::map<int32, std::vector<GLfloat>> normals;
		std::map<int32, std::vector<GLfloat>> texcoords;
		std::map<int32, std::vector<GLuint>> indices;
		std::set<int32> materialIDs;

		bool containsValidNormals(int32 key) const {
			auto x = positions.find(key);
			auto y = normals.find(key);
			return x != positions.end()
				&& y != normals.end()
				&& x->second.size() == y->second.size();
		}
	};

	// Wavefront OBJ
	class OBJLoader {

	public:
		OBJLoader() = default;
		OBJLoader(const OBJLoader& other) = delete;
		OBJLoader(OBJLoader&& rhs) = delete;

		// Should be called before load()
		void setMaterialOverrides(const std::vector<std::pair<std::string, Material*>>&& overrides);
		GLuint findGLTexture(const std::string& textureName) const;

		// Load Wavefront OBJ file and prepare for GPU upload.
		// Can be called from worker threads.
		// NOTE: Actual GPU resources are created later in craftMesh().
		bool load(const char* inObjFile, const char* inMtlDir);
		void unload();

		inline const std::string& getSourceFilepath() const { return objFile; }
		inline bool isValid() const { return bIsValid; }

		inline uint32 numShapes() const { return static_cast<uint32>(pendingShapes.size()); }
		inline const std::string& getShapeName(uint32 index) const { return tiny_shapes[index].name; }
		
		// CAUTION: Must be called in render thread
		Mesh* craftMeshFrom(const std::string& shapeName);

		// CAUTION: Must be called in render thread
		Mesh* craftMeshFrom(uint32 shapeIndex);

		// CAUTION: Must be called in render thread
		Mesh* craftMeshFromAllShapes(bool bMergeShapesIfSameMaterial = false);

		const std::vector<Material*>& getMaterials() { return materials; }

	protected:
		void analyzeMaterials();
		void reconstructShapes();
		Material* getMaterial(int32 index);
		Mesh* craftMesh(uint32 from, uint32 to, bool bMergeShapesIfSameMaterial = false); // both inclusive

	private:
		std::string objFile;
		std::string mtlDir;
		bool bIsValid = false;

		std::vector<tinyobj::shape_t> tiny_shapes;
		std::vector<tinyobj::material_t> tiny_materials;
		tinyobj::attrib_t tiny_attrib;

		std::vector<PendingShape> pendingShapes;
		std::vector<Material*> materials;
		std::vector<std::pair<std::string, Material*>> materialOverrides;
		// Fallback material if there's no matching material for that within .mtl
		Material* defaultMaterial = nullptr;

		std::map<std::string, BitmapBlob*> cachedBitmapDB;
		std::map<int32, PendingTextures> pendingTextureData; // key: material index

		// Can be queried to get source images.
		std::map<std::string, GLuint> glTextureMap;

	};

}
