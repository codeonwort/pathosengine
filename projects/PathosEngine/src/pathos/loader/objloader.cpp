#include "pathos/engine.h"
#include "pathos/rhi/texture.h"
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
#define VERBOSE_LOG                 0

vector3 toVec3(const tinyobj::real_t tiny_real[3]) {
	return vector3(tiny_real[0], tiny_real[1], tiny_real[2]);
}

// https://learn.microsoft.com/en-us/azure/remote-rendering/reference/material-mapping
float PhongSpecularToRoughness(const vector3& specularPower, float shininess) {
	float intensity = (specularPower.x + specularPower.y + specularPower.z) / 3.0f;
	return std::sqrt(2.0f / (shininess * intensity + 2.0f));
}

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

	OBJLoader::~OBJLoader() {
		unload();
	}

	void OBJLoader::setMaterialOverrides(const std::vector<std::pair<std::string, Material*>>&& overrides) {
		materialOverrides = overrides;
	}

	Texture* OBJLoader::findTexture(const std::string& textureName) const {
		auto it = glTextureMap.find(textureName);
		CHECK(it != glTextureMap.end());
		return it->second;
	}

	bool OBJLoader::load(const char* inObjFile, const char* inMtlDir) {
		objFile = ResourceFinder::get().find(inObjFile);
		mtlDir = ResourceFinder::get().find(inMtlDir);

		LOG(LogInfo, "Loading .obj file: %s", objFile.data());

		// Read data using tinyobjloader
		std::string warn, err;
		bool bLoadSuccessful = tinyobj::LoadObj(
			&tiny_attrib, &tiny_shapes, &tiny_materials, &warn, &err,
			objFile.c_str(), mtlDir.c_str());

		if (!warn.empty()) {
			LOG(LogWarning, "Warning while loading OBJ file: %s", warn.data());
		}
		if (!err.empty()) {
			LOG(LogError, "Error while loading OBJ file: %s", err.data());
		}
		if (!bLoadSuccessful) {
			bIsValid = false;
			return bIsValid;
		}
		LOG(LogInfo, "    Number of shapes: %d", (int32)tiny_shapes.size());
		LOG(LogInfo, "    Number of materials: %d", (int32)tiny_materials.size());

		analyzeMaterials();
		reconstructShapes();

		bIsValid = true;
		return bIsValid;
	}

	void OBJLoader::unload() {
		if (bUnloaded) {
			return;
		}
		bUnloaded = true;

		std::set<ImageBlob*> blobsToDestroy;
		for (auto& it : pendingTextureData) {
			if (it.second.albedoBlob != nullptr) blobsToDestroy.insert(it.second.albedoBlob);
			if (it.second.normalBlob != nullptr) blobsToDestroy.insert(it.second.normalBlob);
			if (it.second.roughnessBlob != nullptr) blobsToDestroy.insert(it.second.roughnessBlob);
			if (it.second.metallicBlob != nullptr) blobsToDestroy.insert(it.second.metallicBlob);
		}
		ENQUEUE_DEFERRED_RENDER_COMMAND([blobsToDestroy](RenderCommandList& cmdList) {
			for (ImageBlob* blob : blobsToDestroy) {
				cmdList.registerDeferredCleanup(blob);
			}
		});

		mtlDir.clear();
		tiny_shapes.clear();
		tiny_materials.clear();
		tiny_attrib.normals.clear();
		tiny_attrib.texcoords.clear();
		tiny_attrib.vertices.clear();
		pendingShapes.clear();
		materials.clear();
		cachedImageDB.clear();
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
				auto getOrLoadImage = [&](const std::string& texname, ImageBlob** outBitmap) {
					*outBitmap = nullptr;
					if (texname.length() > 0) {
						std::string filepath = mtlDir + texname;
						if (cachedImageDB.find(filepath) == cachedImageDB.end()) {
							*outBitmap = pathos::ImageUtils::loadImage(filepath.c_str());
							cachedImageDB.insert(std::make_pair(filepath, *outBitmap));
						} else {
							*outBitmap = cachedImageDB[filepath];
						}
					}
				};

				// File loading is performed in worker threads. Actual GL textures are created later.
				PendingTextures pending;
				getOrLoadImage(t_mat.diffuse_texname, &pending.albedoBlob);
				getOrLoadImage(t_mat.normal_texname, &pending.normalBlob);
				getOrLoadImage(t_mat.roughness_texname, &pending.roughnessBlob);
				getOrLoadImage(t_mat.metallic_texname, &pending.metallicBlob);
				pending.albedoFilename = t_mat.diffuse_texname;
				pending.normalFilename = t_mat.normal_texname;
				pending.roughnessFilename = t_mat.roughness_texname;
				pending.metallicFilename = t_mat.metallic_texname;
				// #todo-loader: Self AO texture is not supported in Wavefront format

				std::string image_path = mtlDir + t_mat.diffuse_texname;
				ImageBlob* bmp;
				if (cachedImageDB.find(image_path) == cachedImageDB.end()) {
					bmp = pathos::ImageUtils::loadImage(image_path.c_str());
					cachedImageDB.insert(std::make_pair(image_path, bmp));
				} else {
					bmp = cachedImageDB[image_path];
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

				float roughness = t_mat.roughness;
				if (roughness <= 0.0f) {
					vector3 specular = toVec3(t_mat.specular);
					roughness = PhongSpecularToRoughness(specular, t_mat.shininess);
				}
				
				// Ignore ambient term and let GI system handle indirect illumination.
 				//solidColor->setAmbient(t_mat.ambient[0], t_mat.ambient[1], t_mat.ambient[2]);

				M->setConstantParameter("albedo", vector3(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]));
				M->setConstantParameter("metallic", t_mat.metallic);
				M->setConstantParameter("roughness", roughness);
				M->setConstantParameter("emissive", vector3(t_mat.emission[0], t_mat.emission[1], t_mat.emission[2]));
			}

			if (overrideIx != -1) {
				// #todo-loader: Would be best not to create it at first...
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
			const tinyobj::mesh_t& srcMesh = tiny_shapes[i].mesh;
			PendingShape dst;

			// int32 = face material id
			// uint32 = vertex index
			std::map<int32, std::unordered_map<MetaVertex, uint32>> uniqueVertices;
			std::map<int32, std::vector<MetaVertex>> metaVertices;
			std::set<int32> normalInvalidFlags;

#if VERBOSE_LOG
			LOG(LogDebug, "Analyzing OBJ shape[%d]: %s", i, src.name.data());
#endif

			for (size_t f = 0; f < srcMesh.num_face_vertices.size(); ++f) {
				int32 faceMaterialID = (int32)(srcMesh.material_ids[f]);
#if WARN_INVALID_FACE_MARTERIAL
				CHECK(faceMaterialID >= 0); // invalid material id
#endif
				dst.materialIDs.insert(faceMaterialID);
			}

			size_t numMaterials = dst.materialIDs.size();
			size_t index_offset = 0;

#if VERBOSE_LOG
			LOG(LogDebug, "Number of materials for this shape: %d", numMaterials);
#endif

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
#if VERBOSE_LOG
					LOG(LogWarning, "(Material ID = %d) Vertex normals are invalid and will be recalculated", mat);
#endif
				}
			}

			pendingShapes.emplace_back(dst);

#if VERBOSE_LOG
			LOG(LogDebug, "Shape has been parsed");
#endif
		}
	}

	StaticMesh* OBJLoader::craftMeshFrom(const std::string& shapeName) {
		for (size_t i = 0; i < tiny_shapes.size(); ++i) {
			if (tiny_shapes[i].name == shapeName) {
				return craftMeshFrom(static_cast<uint32>(i));
			}
		}
		return nullptr;
	}
	StaticMesh* OBJLoader::craftMeshFrom(uint32 shapeIndex) {
		return craftMesh(shapeIndex, shapeIndex);
	}
	StaticMesh* OBJLoader::craftMeshFromAllShapes(bool bMergeShapesIfSameMaterial) {
		return craftMesh(0, static_cast<uint32>(pendingShapes.size() - 1), bMergeShapesIfSameMaterial);
	}

	StaticMesh* OBJLoader::craftMesh(uint32 from, uint32 to, bool bMergeShapesIfSameMaterial) {
		CHECK(0 <= from && from < pendingShapes.size());
		CHECK(0 <= to && to < pendingShapes.size());
		CHECK(from <= to);

		StaticMesh* mesh = new StaticMesh;

		if (bMergeShapesIfSameMaterial) {
			std::map<int32, std::vector<uint32>> materialToShapes;
			for (uint32 shapeIx = from; shapeIx <= to; ++shapeIx) {
				const PendingShape& shape = pendingShapes[shapeIx];
				for (int32 materialID : shape.materialIDs) {
					materialToShapes[materialID].push_back(shapeIx);
				}
			}
			for (auto it = materialToShapes.begin(); it != materialToShapes.end(); ++it) {
				const int32 materialID = it->first;
				const std::vector<uint32>& shapeIxArray = it->second;

				// Merged buffers
				std::vector<float> positions, normals, texcoords;
				std::vector<uint32> indices;
				bool bValidNormal = true;
				for (uint32 shapeIx : shapeIxArray) {
					bValidNormal = bValidNormal && pendingShapes[shapeIx].containsValidNormals(materialID);
				}

#if 0
				size_t posLength = 0, normLength = 0, texLength = 0, ixLength = 0;
				for (uint32 shapeIx : shapeIxArray) {
					posLength += pendingShapes[shapeIx].positions[materialID].size();
					normLength += pendingShapes[shapeIx].normals[materialID].size();
					texLength += pendingShapes[shapeIx].texcoords[materialID].size();
					ixLength += pendingShapes[shapeIx].indices[materialID].size();
				}

				positions.resize(posLength);
				normals.resize(normLength);
				texcoords.resize(texLength);
				indices.resize(ixLength);

				posLength = normLength = texLength = ixLength = 0;
				for (uint32 shapeIx : shapeIxArray) {
					auto& partPositions = pendingShapes[shapeIx].positions[materialID];
					auto& partNormals = pendingShapes[shapeIx].normals[materialID];
					auto& partTexcoords = pendingShapes[shapeIx].texcoords[materialID];
					auto& partIndices = pendingShapes[shapeIx].indices[materialID];
					positions.insert(positions.begin() + posLength, partPositions.begin(), partPositions.end());
					normals.insert(normals.begin() + normLength, partNormals.begin(), partNormals.end());
					texcoords.insert(texcoords.begin() + texLength, partTexcoords.begin(), partTexcoords.end());
					indices.insert(indices.begin() + ixLength, partIndices.begin(), partIndices.end());
					for (size_t ix = ixLength; ix < ixLength + partIndices.size(); ++ix) {
						indices[ix] += (uint32)(posLength / 3);
					}
					posLength += partPositions.size();
					normLength += partNormals.size();
					texLength += partTexcoords.size();
					ixLength += partIndices.size();
				}
#else
				std::unordered_map<MetaVertex, uint32> uniqueVertices;
				std::vector<MetaVertex> metaVertices;
				for (uint32 shapeIx : shapeIxArray) {
					auto& partPositions = pendingShapes[shapeIx].positions[materialID];
					auto& partNormals = pendingShapes[shapeIx].normals[materialID];
					auto& partTexcoords = pendingShapes[shapeIx].texcoords[materialID];
					auto& partIndices = pendingShapes[shapeIx].indices[materialID];

					for (uint32 i : partIndices) {
						MetaVertex metaV;
						metaV.position = vector3(partPositions[i * 3 + 0], partPositions[i * 3 + 1], partPositions[i * 3 + 2]);
						metaV.texcoord = vector2(partTexcoords[i * 2 + 0], partTexcoords[i * 2 + 1]);
						if (bValidNormal) {
							metaV.normal = vector3(partNormals[i * 3 + 0], partNormals[i * 3 + 1], partNormals[i * 3 + 2]);
						} else {
							metaV.normal = vector3(0.0f);
						}

						if (uniqueVertices.count(metaV) == 0) {
							uniqueVertices[metaV] = (uint32)metaVertices.size();
							metaVertices.push_back(metaV);

							positions.push_back(metaV.position.x);
							positions.push_back(metaV.position.y);
							positions.push_back(metaV.position.z);
							texcoords.push_back(metaV.texcoord.x);
							texcoords.push_back(metaV.texcoord.y);
							normals.push_back(metaV.normal.x);
							normals.push_back(metaV.normal.y);
							normals.push_back(metaV.normal.z);
						}
						indices.push_back(uniqueVertices[metaV]);
					}
				}
#endif

				MeshGeometry* geom = new MeshGeometry;
				geom->initializeVertexLayout(MeshGeometry::EVertexAttributes::All);
				geom->updatePositionData(&positions[0], static_cast<uint32>(positions.size()));
				geom->updateUVData(&texcoords[0], static_cast<uint32>(texcoords.size()));
				geom->updateIndexData(&indices[0], static_cast<uint32>(indices.size()));
				if (bValidNormal) {
					geom->updateNormalData(&normals[0], static_cast<uint32>(normals.size()));
				} else {
					geom->calculateNormals();
				}
				geom->calculateTangentBasis();

				mesh->addSection(0, geom, getMaterial(materialID));
			}
		} else {
			for (uint32 i = from; i <= to; ++i) {
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
					geom->initializeVertexLayout(MeshGeometry::EVertexAttributes::All);
					geom->updatePositionData(&positions[0], static_cast<uint32>(positions.size()));
					geom->updateUVData(&texcoords[0], static_cast<uint32>(texcoords.size()));
					geom->updateIndexData(&indices[0], static_cast<uint32>(indices.size()));
					if (shape.containsValidNormals(materialID)) {
						geom->updateNormalData(&normals[0], static_cast<uint32>(normals.size()));
					} else {
						geom->calculateNormals();
					}
					geom->calculateTangentBasis();

					mesh->addSection(0, geom, getMaterial(materialID));
				}
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
			constexpr uint32 mipLevels = 0;
			constexpr bool sRGB = true;
			constexpr bool autoDestroy = false;
			PendingTextures& pendingTextures = pendingTextureData[index];
			if (pendingTextures.albedoTexture == nullptr && pendingTextures.albedoBlob != nullptr) {
				pendingTextures.albedoTexture = pathos::ImageUtils::createTexture2DFromImage(pendingTextures.albedoBlob, mipLevels, sRGB, autoDestroy, pendingTextures.albedoFilename.c_str());
				glTextureMap[pendingTextures.albedoFilename] = pendingTextures.albedoTexture;
			}
			if (pendingTextures.normalTexture == nullptr && pendingTextures.normalBlob != nullptr) {
				pendingTextures.normalTexture = pathos::ImageUtils::createTexture2DFromImage(pendingTextures.normalBlob, mipLevels, !sRGB, autoDestroy, pendingTextures.normalFilename.c_str());
				glTextureMap[pendingTextures.normalFilename] = pendingTextures.normalTexture;
			}
			if (pendingTextures.roughnessTexture == nullptr && pendingTextures.roughnessBlob != nullptr) {
				pendingTextures.roughnessTexture = pathos::ImageUtils::createTexture2DFromImage(pendingTextures.roughnessBlob, mipLevels, !sRGB, autoDestroy, pendingTextures.roughnessFilename.c_str());
				glTextureMap[pendingTextures.roughnessFilename] = pendingTextures.roughnessTexture;
			}
			if (pendingTextures.metallicTexture == nullptr && pendingTextures.metallicBlob != nullptr) {
				pendingTextures.metallicTexture = pathos::ImageUtils::createTexture2DFromImage(pendingTextures.metallicBlob, mipLevels, !sRGB, autoDestroy, pendingTextures.metallicFilename.c_str());
				glTextureMap[pendingTextures.metallicFilename] = pendingTextures.metallicTexture;
			}
			
			if (M->getMaterialName() == "pbr_texture") {
				PendingTextures& pendingTextures = pendingTextureData[index];
				if (pendingTextures.albedoBlob != nullptr) {
					GLenum fmt = pendingTextures.albedoBlob->glPixelFormat;
					M->setConstantParameter("bHasOpacity", fmt == GL_RGBA || fmt == GL_BGRA);
				}
				if (pendingTextures.albedoTexture != nullptr) M->setTextureParameter("albedo", pendingTextures.albedoTexture);
				if (pendingTextures.normalTexture != nullptr) M->setTextureParameter("normal", pendingTextures.normalTexture);
				if (pendingTextures.roughnessTexture != nullptr) (M)->setTextureParameter("roughness", pendingTextures.roughnessTexture);
				if (pendingTextures.metallicTexture != nullptr) M->setTextureParameter("metallic", pendingTextures.metallicTexture);
			}
		}

		return M;
	}

}
