#include "pathos/engine.h"
#include "pathos/loader/objloader.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include <unordered_map>
#include <type_traits>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// 0: Just use fallback material
// 1: Assert if invalid material
#define WARN_INVALID_FACE_MARTERIAL 0

// Vertex deduplication: https://vulkan-tutorial.com/Loading_models
struct MetaVertex {
	vector3 position;
	vector2 texcoord;
	vector3 normal;

	MetaVertex() = default;

	bool operator==(const MetaVertex& other) const {
		return position == other.position
			&& texcoord == other.texcoord
			&& normal == other.normal;
	}
};
namespace std {
	template<> struct std::hash<MetaVertex> {
		size_t operator()(MetaVertex const& vertex) const {
			return (
				std::hash<vector3>()(vertex.position) ^
				(std::hash<vector2>()(vertex.texcoord) << 1) ^
				((std::hash<vector3>()(vertex.normal) << 1) >> 1));
		}
	};
}

namespace pathos {

	void OBJLoader::setMaterialOverrides(const std::vector<std::pair<std::string, Material*>>&& overrides) {
		materialOverrides = overrides;
	}

	GLuint OBJLoader::findGLTexture(const std::string& textureName) const {
		auto it = glTextureMap.find(textureName);
		CHECK(it != glTextureMap.end());
		return it->second;
	}

	bool OBJLoader::load(const char* inObjFile, const char* inMtlDir) {
		objFile = ResourceFinder::get().find(inObjFile);
		mtlDir = ResourceFinder::get().find(inMtlDir);

		// Read data using tinyobjloader
		std::string err;
		bool loaded = tinyobj::LoadObj(
			&tiny_attrib, &tiny_shapes, &tiny_materials, &err,
			objFile.c_str(), mtlDir.c_str());

		LOG(LogInfo, "Loading .obj file: %s", objFile.data());
		if (!err.empty()) {
			LOG(LogError, "Error while loading OBJ file: %s", err.data());
			if (!loaded) {
				bIsValid = false;
				return bIsValid;
			}
		}
		LOG(LogInfo, "Number of shapes: %d", (int32)tiny_shapes.size());
		LOG(LogInfo, "Number of materials: %d", (int32)tiny_materials.size());

		analyzeMaterials();
		reconstructShapes();

		bIsValid = true;
		return bIsValid;
	}

	void OBJLoader::unload() {
		mtlDir.clear();
		tiny_shapes.clear();
		tiny_materials.clear();
		tiny_attrib.normals.clear();
		tiny_attrib.texcoords.clear();
		tiny_attrib.vertices.clear();
		pendingShapes.clear();
		materials.clear();
		cachedBitmapDB.clear();
		pendingTextureData.clear();
	}

	void OBJLoader::analyzeMaterials() {
		for (size_t i = 0; i < tiny_materials.size(); i++) {
			tinyobj::material_t& t_mat = tiny_materials[i];
			Material* M = nullptr;

			int32 overrideIx = -1;
			for (size_t k = 0; k < materialOverrides.size(); ++k) {
				if (materialOverrides[k].first == t_mat.name) {
					overrideIx = (int32)k;
					break;
				}
			}

			// #todo-loader: More robust criteria
			bool isPBR = t_mat.diffuse_texname.length() > 0;

			if (isPBR)
			{
				auto getOrLoadImage = [&](const std::string& texname, BitmapBlob** outBitmap) {
					*outBitmap = nullptr;
					if (texname.length() > 0) {
						std::string filepath = mtlDir + texname;
						if (cachedBitmapDB.find(filepath) == cachedBitmapDB.end()) {
							*outBitmap = pathos::loadImage(filepath.c_str());
							cachedBitmapDB.insert(std::make_pair(filepath, *outBitmap));
						} else {
							*outBitmap = cachedBitmapDB[filepath];
						}
					}
				};

				// File loading is performed in worker threads. Actual GL textures are created later.
				PendingTextures pending;
				getOrLoadImage(t_mat.diffuse_texname, &pending.albedo);
				getOrLoadImage(t_mat.normal_texname, &pending.normal);
				getOrLoadImage(t_mat.roughness_texname, &pending.roughness);
				getOrLoadImage(t_mat.metallic_texname, &pending.metallic);
				pending.albedoFilename = t_mat.diffuse_texname;
				pending.normalFilename = t_mat.normal_texname;
				pending.roughnessFilename = t_mat.roughness_texname;
				pending.metallicFilename = t_mat.metallic_texname;
				// #todo-loader: Self AO texture is not supported in Wavefront format

				std::string image_path = mtlDir + t_mat.diffuse_texname;
				BitmapBlob* bmp;
				if (cachedBitmapDB.find(image_path) == cachedBitmapDB.end()) {
					bmp = pathos::loadImage(image_path.c_str());
					cachedBitmapDB.insert(std::make_pair(image_path, bmp));
				} else {
					bmp = cachedBitmapDB[image_path];
				}

				M = pathos::createPBRMaterial(gEngine->getSystemTexture2DGrey());
				pendingTextureData.insert(std::make_pair(static_cast<int32>(i), pending));
			}
			else if (t_mat.dissolve < 1.0f
				|| (0.0f <= t_mat.transmittance[0] && t_mat.transmittance[0] < 1.0f)
				|| (0.0f <= t_mat.transmittance[1] && t_mat.transmittance[1] < 1.0f)
				|| (0.0f <= t_mat.transmittance[2] && t_mat.transmittance[2] < 1.0f))
			{
				M = Material::createMaterialInstance("translucent_color");
				M->setConstantParameter("albedo", vector3(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]));
				M->setConstantParameter("roughness", t_mat.roughness);
				M->setConstantParameter("transmittance", vector3(t_mat.transmittance[0], t_mat.transmittance[1], t_mat.transmittance[2]));
			}
			else
			{
				M = Material::createMaterialInstance("solid_color");
				
				// #todo-loader: What to do with ambient and specular
 				//solidColor->setAmbient(t_mat.ambient[0], t_mat.ambient[1], t_mat.ambient[2]);
 				//solidColor->setSpecular(t_mat.specular[0], t_mat.specular[1], t_mat.specular[2]);
				M->setConstantParameter("albedo", vector3(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]));
				M->setConstantParameter("metallic", t_mat.metallic);
				M->setConstantParameter("roughness", t_mat.roughness);
				M->setConstantParameter("emissive", vector3(t_mat.emission[0], t_mat.emission[1], t_mat.emission[2]));
			}

			if (overrideIx != -1) {
				// #todo: Would be best not to create it at first...
				if (M != nullptr) {
					delete M;
				}
				M = materialOverrides[overrideIx].second;
			}

			materials.push_back(M);
		}

		// used for shapes whose material id is invalid
		defaultMaterial = Material::createMaterialInstance("solid_color");
		defaultMaterial->setConstantParameter("albedo", vector3(0.0f, 0.9f, 0.0f));
		defaultMaterial->setConstantParameter("metallic", 0.0f);
		defaultMaterial->setConstantParameter("roughness", 0.9f);
		defaultMaterial->setConstantParameter("emissive", vector3(0.0f));
	}

	void OBJLoader::reconstructShapes() {
		pendingShapes.clear();

		for (size_t i = 0; i < tiny_shapes.size(); ++i) {
			const tinyobj::shape_t& src = tiny_shapes[i];
			const tinyobj::mesh_t& srcMesh = src.mesh;
			PendingShape dst;

			// int32 = face material id
			// uint32 = vertex index
			std::map<int32, std::unordered_map<MetaVertex, uint32>> uniqueVertices;
			std::map<int32, std::vector<MetaVertex>> metaVertices;
			std::set<int32> normalInvalidFlags;

			LOG(LogDebug, "Analyzing OBJ shape[%d]: %s", i, src.name.data());

			for (size_t f = 0; f < srcMesh.num_face_vertices.size(); ++f) {
				int32 faceMaterialID = (int32)(srcMesh.material_ids[f]);
#if WARN_INVALID_FACE_MARTERIAL
				CHECK(faceMaterialID >= 0); // invalid material id
#endif
				dst.materialIDs.insert(faceMaterialID);
			}

			size_t numMaterials = dst.materialIDs.size();
			size_t index_offset = 0;

			LOG(LogDebug, "Number of materials for this shape: %d", numMaterials);

			for (size_t f = 0u; f < srcMesh.num_face_vertices.size(); ++f) {
				int32 fv = srcMesh.num_face_vertices[f];
				CHECK(fv == 3);

				int32 materialID = srcMesh.material_ids[f];

				std::vector<GLfloat>& positionBuffer = dst.positions[materialID];
				std::vector<GLfloat>& texcoordBuffer = dst.texcoords[materialID];
				std::vector<GLfloat>& normalBuffer = dst.normals[materialID];
				std::vector<GLuint>& indexBuffer = dst.indices[materialID];

				for (int32 v = 0; v < fv; ++v) {
					tinyobj::index_t idx = srcMesh.indices[index_offset + v];

					// position data (should exist)
					float posX = tiny_attrib.vertices[3 * idx.vertex_index + 0];
					float posY = tiny_attrib.vertices[3 * idx.vertex_index + 1];
					float posZ = tiny_attrib.vertices[3 * idx.vertex_index + 2];

					// texcoord data (optional)
					float texU = 0.0f;
					float texV = 0.0f;
					if (idx.texcoord_index >= 0) {
						texU = tiny_attrib.texcoords[2 * idx.texcoord_index + 0];
						texV = tiny_attrib.texcoords[2 * idx.texcoord_index + 1];
					}

					// normal data (optional)
					float nx = 0.0f, ny = 0.0f, nz = 0.0f;
					if (idx.normal_index >= 0) {
						nx = tiny_attrib.normals[3 * idx.normal_index + 0];
						ny = tiny_attrib.normals[3 * idx.normal_index + 1];
						nz = tiny_attrib.normals[3 * idx.normal_index + 2];
					} else {
						normalInvalidFlags.insert(materialID);
					}

					MetaVertex metaV;
					metaV.position = vector3(posX, posY, posZ);
					metaV.texcoord = vector2(texU, texV);
					metaV.normal = vector3(nx, ny, nz);

					if (uniqueVertices[materialID].count(metaV) == 0) {
						uniqueVertices[materialID][metaV] = (uint32)metaVertices[materialID].size();
						metaVertices[materialID].push_back(metaV);

						positionBuffer.push_back(posX);
						positionBuffer.push_back(posY);
						positionBuffer.push_back(posZ);
						texcoordBuffer.push_back(texU);
						texcoordBuffer.push_back(texV);
						normalBuffer.push_back(nx);
						normalBuffer.push_back(ny);
						normalBuffer.push_back(nz);
					}

					indexBuffer.push_back(uniqueVertices[materialID][metaV]);
				}
				index_offset += fv;
			}

			for (const auto& it : dst.normals) {
				int32 mat = it.first;
				if (normalInvalidFlags.find(mat) != normalInvalidFlags.end()) {
					dst.normals[mat].clear();
					LOG(LogWarning, "(Material ID = %d) Vertex normals are invalid and will be recalculated", mat);
				}
			}

			pendingShapes.emplace_back(dst);

			LOG(LogDebug, "Shape has been parsed");
		}
	}

	Mesh* OBJLoader::craftMeshFrom(const std::string& shapeName) {
		for (size_t i = 0; i < tiny_shapes.size(); ++i) {
			if (tiny_shapes[i].name == shapeName) {
				return craftMeshFrom(static_cast<uint32>(i));
			}
		}
		return nullptr;
	}
	Mesh* OBJLoader::craftMeshFrom(uint32 shapeIndex) {
		return craftMesh(shapeIndex, shapeIndex);
	}
	Mesh* OBJLoader::craftMeshFromAllShapes() {
		return craftMesh(0, static_cast<uint32>(pendingShapes.size() - 1));
	}

	Mesh* OBJLoader::craftMesh(uint32 from, uint32 to) {
		CHECK(0 <= from && from < pendingShapes.size());
		CHECK(0 <= to && to < pendingShapes.size());
		CHECK(from <= to);

		Mesh* mesh = new Mesh;

		for (unsigned int i = from; i <= to; ++i) {
			PendingShape& shape = pendingShapes[i];

			for (int32 materialID : shape.materialIDs) {
#if WARN_INVALID_FACE_MARTERIAL
				CHECK(materialID >= 0);
#endif
				auto& positions = shape.positions[materialID];
				auto& normals = shape.normals[materialID];
				auto& texcoords = shape.texcoords[materialID];
				auto& indices = shape.indices[materialID];

				MeshGeometry* geom = new MeshGeometry;
				geom->setDrawArraysMode(false);
				geom->updatePositionData(&positions[0], static_cast<uint32>(positions.size()));
				geom->updateUVData(&texcoords[0], static_cast<uint32>(texcoords.size()));
				geom->updateIndexData(&indices[0], static_cast<uint32>(indices.size()));
				if (shape.containsValidNormals(materialID)) {
					geom->updateNormalData(&normals[0], static_cast<uint32>(normals.size()));
				} else {
					geom->calculateNormals();
				}
				geom->calculateTangentBasis();

				mesh->add(geom, getMaterial(materialID));
			}
		}

		return mesh;
	}

	Material* OBJLoader::getMaterial(int32 index) {
		CHECK(-1 <= index && index < (int32)materials.size());
		if (index == -1) {
			return defaultMaterial;
		}

		Material* M = materials[index];

		if (pendingTextureData.find(index) != pendingTextureData.end()) {
			constexpr bool generateMipmap = true;
			constexpr bool sRGB = true;
			PendingTextures& pendingTextures = pendingTextureData[index];
			if (pendingTextures.glAlbedo == 0 && pendingTextures.albedo != nullptr) {
				pendingTextures.glAlbedo = pathos::createTextureFromBitmap(pendingTextures.albedo, generateMipmap, sRGB);
				glTextureMap[pendingTextures.albedoFilename] = pendingTextures.glAlbedo;
			}
			if (pendingTextures.glNormal == 0 && pendingTextures.normal != nullptr) {
				pendingTextures.glNormal = pathos::createTextureFromBitmap(pendingTextures.normal, generateMipmap, !sRGB);
				glTextureMap[pendingTextures.normalFilename] = pendingTextures.glNormal;
			}
			if (pendingTextures.glRoughness == 0 && pendingTextures.roughness != nullptr) {
				pendingTextures.glRoughness = pathos::createTextureFromBitmap(pendingTextures.roughness, generateMipmap, !sRGB);
				glTextureMap[pendingTextures.roughnessFilename] = pendingTextures.glRoughness;
			}
			if (pendingTextures.glMetallic == 0 && pendingTextures.metallic != nullptr) {
				pendingTextures.glMetallic = pathos::createTextureFromBitmap(pendingTextures.metallic, generateMipmap, !sRGB);
				glTextureMap[pendingTextures.metallicFilename] = pendingTextures.glMetallic;
			}

			if (M->getMaterialName() == "pbr_texture") {
				PendingTextures& pendingTextures = pendingTextureData[index];
				if (pendingTextures.albedo != nullptr) {
					M->setConstantParameter("bHasOpacity", pendingTextures.albedo->hasOpacity);
				}
				if (pendingTextures.glAlbedo != 0) M->setTextureParameter("albedo", pendingTextures.glAlbedo);
				if (pendingTextures.glNormal != 0) M->setTextureParameter("normal", pendingTextures.glNormal);
				if (pendingTextures.glRoughness != 0) (M)->setTextureParameter("roughness", pendingTextures.glRoughness);
				if (pendingTextures.glMetallic != 0) M->setTextureParameter("metallic", pendingTextures.glMetallic);
			}
		}

		return M;
	}

}
