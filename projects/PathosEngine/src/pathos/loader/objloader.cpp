#include "pathos/engine.h"
#include "pathos/loader/objloader.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include <unordered_map>
#include <type_traits>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct MetaVertex {
	vector3 position;
	vector2 texcoord;

	MetaVertex() = default;

	bool operator==(const MetaVertex& other) const {
		return position == other.position && texcoord == other.texcoord;
	}
};

namespace std {
	template<> struct std::hash<MetaVertex> {
		size_t operator()(MetaVertex const& vertex) const {
			return
				((std::hash<vector3>()(vertex.position) >> 1) ^
				(std::hash<vector2>()(vertex.texcoord) << 1));
		}
	};
}

#define LOAD_NORMAL_DATA            0
#define WARN_INVALID_FACE_MARTERIAL 0
#define LOAD_AS_PBR_TEXTURE         1

namespace pathos {

	// #todo: Same as that in geometry.cpp
	void calculateNormal(const std::vector<GLfloat>& positions, const std::vector<GLuint>& indices, std::vector<GLfloat>& outNormals) {
		CHECK(indices.size() % 3 == 0);
		outNormals.resize(positions.size());

		const uint32 numPos = (uint32)(positions.size() / 3);
		std::vector<vector3> accum(numPos, vector3(0.0f, 0.0f, 0.0f));

		const auto& P = positions;

		for (size_t i = 0u; i < indices.size(); i += 3) {
			GLuint i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
			GLuint p0 = i0 * 3, p1 = i1 * 3, p2 = i2 * 3;
			vector3 a = vector3(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			vector3 b = vector3(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);

			// #todo-loader: Well... we have a problem.
			if (a == b || i0 == i1 || i1 == i2 || i2 == i0) {
				continue;
			}

			vector3 norm = glm::normalize(glm::cross(a, b));
			if (isnan(norm.x) || isnan(norm.y) || isnan(norm.z)) {
				continue;
			}

			accum[i0] += norm;
			accum[i1] += norm;
			accum[i2] += norm;
		}
		const vector3 zero(0.0f);
		for (uint32 i = 0u; i < accum.size(); i++) {
			vector3 N = glm::normalize(accum[i]);
			if (accum[i] == zero) {
				N = vector3(0, 1, 0);
			}
			outNormals[i * 3 + 0] = N.x;
			outNormals[i * 3 + 1] = N.y;
			outNormals[i * 3 + 2] = N.z;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////

	OBJLoader::OBJLoader()
		: bIsValid(false)
	{
	}

	OBJLoader::OBJLoader(const char* inObjFile, const char* inMtlDir) {
		load(inObjFile, inMtlDir);
	}

	bool OBJLoader::load(const char* inObjFile, const char* inMtlDir) {
		objFile = ResourceFinder::get().find(inObjFile);
		mtlDir = ResourceFinder::get().find(inMtlDir);

		// Read data using tinyobjloader
		std::string err;
		bool loaded = tinyobj::LoadObj(
			&t_attrib, &t_shapes, &t_materials, &err,
			objFile.c_str(), mtlDir.c_str());

		LOG(LogInfo, "Loading .obj file: %s", objFile.data());
		if (!err.empty()) {
			LOG(LogError, "Error while loading OBJ file: %s", err.data());
			if (!loaded) {
				bIsValid = false;
				return bIsValid;
			}
		}
		LOG(LogInfo, "Number of shapes: %d", (int32)t_shapes.size());
		LOG(LogInfo, "Number of materials: %d", (int32)t_materials.size());

		// Reconstruct meshes and materials
		analyzeMaterials(t_materials, materials);
		reconstructShapes(t_shapes, t_attrib, pendingShapes);

		bIsValid = true;
		return bIsValid;
	}

	void OBJLoader::unload() {
		mtlDir.clear();
		t_shapes.clear();
		t_materials.clear();
		t_attrib.normals.clear();
		t_attrib.texcoords.clear();
		t_attrib.vertices.clear();
		pendingShapes.clear();
		materials.clear();
		for (auto& bmp : cachedBitmapDB) {
			FreeImage_Unload(bmp.second);
		}
		cachedBitmapDB.clear();
		pendingTextureData.clear();
	}

	void OBJLoader::analyzeMaterials(const std::vector<tinyobj::material_t>& tiny_materials, std::vector<Material*>& output) {
		for (size_t i = 0; i < t_materials.size(); i++) {
			tinyobj::material_t& t_mat = t_materials[i];
			Material* M = nullptr;

			// #todo-loader: More robust criteria
			bool isPBR = t_mat.diffuse_texname.length() > 0;

			if (isPBR)
			{
				auto getOrLoadImage = [&](const std::string& texname, FIBITMAP** outBitmap) {
					*outBitmap = nullptr;
					if (texname.length() > 0) {
						std::string filepath = mtlDir + texname;
						if (cachedBitmapDB.find(filepath) == cachedBitmapDB.end()) {
							*outBitmap = loadImage(filepath.c_str());
							cachedBitmapDB.insert(std::make_pair(filepath, *outBitmap));
						} else {
							*outBitmap = cachedBitmapDB[filepath];
						}
					}
				};

				PendingTextures pending;
				getOrLoadImage(t_mat.diffuse_texname, &pending.albedo);
				getOrLoadImage(t_mat.normal_texname, &pending.normal);
				getOrLoadImage(t_mat.roughness_texname, &pending.roughness);
				getOrLoadImage(t_mat.metallic_texname, &pending.metallic);
				// #todo-loader: Self AO texture is not supported in Wavefront format

				std::string image_path = mtlDir + t_mat.diffuse_texname;
				FIBITMAP* bmp;
				if (cachedBitmapDB.find(image_path) == cachedBitmapDB.end()) {
					bmp = loadImage(image_path.c_str());
					cachedBitmapDB.insert(std::make_pair(image_path, bmp));
				} else {
					bmp = cachedBitmapDB[image_path];
				}

				// This is performed in loading thread, so creation of actual GL textures are delayed until getMaterial() call by main thread.
#if LOAD_AS_PBR_TEXTURE
				// https://en.wikipedia.org/wiki/Wavefront_.obj_file#Physically-based_Rendering
				M = new PBRTextureMaterial(
					gEngine->getSystemTexture2DWhite(),  // albedo
					gEngine->getSystemTexture2DBlue(),   // normal
					gEngine->getSystemTexture2DGrey(),   // metallic
					gEngine->getSystemTexture2DWhite(),  // roughness
					gEngine->getSystemTexture2DWhite()); // localAO
#else
				M = new TextureMaterial(0);
#endif
				pendingTextureData.insert(make_pair(static_cast<int32>(i), pending));
			}
			else if (t_mat.dissolve < 1.0f
				|| (0.0f <= t_mat.transmittance[0] && t_mat.transmittance[0] < 1.0f)
				|| (0.0f <= t_mat.transmittance[1] && t_mat.transmittance[1] < 1.0f)
				|| (0.0f <= t_mat.transmittance[2] && t_mat.transmittance[2] < 1.0f))
			{
				TranslucentColorMaterial* translucentColor = new TranslucentColorMaterial;
				translucentColor->setAlbedo(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]);
				translucentColor->setMetallic(t_mat.metallic);
				translucentColor->setRoughness(t_mat.roughness);
				translucentColor->setOpacity(t_mat.dissolve);
				translucentColor->setTransmittance(vector3(t_mat.transmittance[0], t_mat.transmittance[1], t_mat.transmittance[2]));
				
				M = translucentColor;
			}
			else
			{
				ColorMaterial* solidColor = new ColorMaterial;
				
				// #todo-loader: What to do with ambient and specular
 				//solidColor->setAmbient(t_mat.ambient[0], t_mat.ambient[1], t_mat.ambient[2]);
 				//solidColor->setSpecular(t_mat.specular[0], t_mat.specular[1], t_mat.specular[2]);
				solidColor->setAlbedo(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]);
				solidColor->setMetallic(t_mat.metallic);
				solidColor->setRoughness(t_mat.roughness);
				solidColor->setEmissive(t_mat.emission[0], t_mat.emission[1], t_mat.emission[2]);

				M = solidColor;
			}

			M->setName(t_mat.name);
			materials.push_back(M);
		}

		// used for shapes whose material id is invalid
		defaultMaterial = new ColorMaterial;
		ColorMaterial* M = defaultMaterial;
		M->setAlbedo(0.0f, 1.0f, 0.0f);
	}

	void OBJLoader::reconstructShapes(const std::vector<tinyobj::shape_t>& tiny_shapes, const tinyobj::attrib_t& tiny_attrib, std::vector<PendingShape>& outPendingShapes) {
		outPendingShapes.clear();

		for (size_t i = 0; i < tiny_shapes.size(); ++i) {
			const tinyobj::shape_t& src = t_shapes[i];
			const tinyobj::mesh_t& srcMesh = src.mesh;
			PendingShape dst;

			// Vertex deduplication: https://vulkan-tutorial.com/Loading_models
			// int32 = face material id
			// uint32 = vertex index
			std::map<int32, std::unordered_map<MetaVertex, uint32>> uniqueVertices;
			std::map<int32, std::vector<MetaVertex>> metaVertices;

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
				std::vector<GLuint>& indexBuffer = dst.indices[materialID];

				for (int32 v = 0; v < fv; ++v) {
					tinyobj::index_t idx = srcMesh.indices[index_offset + v];

					// position data
					float posX = tiny_attrib.vertices[3 * idx.vertex_index + 0];
					float posY = tiny_attrib.vertices[3 * idx.vertex_index + 1];
					float posZ = tiny_attrib.vertices[3 * idx.vertex_index + 2];

					// texcoord data
					float texU = 0.0f;
					float texV = 0.0f;
					if (idx.texcoord_index >= 0) {
						texU = tiny_attrib.texcoords[2 * idx.texcoord_index + 0];
						texV = tiny_attrib.texcoords[2 * idx.texcoord_index + 1];
					}

					MetaVertex metaV;
					metaV.position = vector3(posX, posY, posZ);
					metaV.texcoord = vector2(texU, texV);

					if (uniqueVertices[materialID].count(metaV) == 0) {
						uniqueVertices[materialID][metaV] = static_cast<uint32>(metaVertices[materialID].size());
						metaVertices[materialID].push_back(metaV);

						positionBuffer.push_back(posX);
						positionBuffer.push_back(posY);
						positionBuffer.push_back(posZ);
						texcoordBuffer.push_back(texU);
						texcoordBuffer.push_back(texV);
					}

					indexBuffer.push_back(uniqueVertices[materialID][metaV]);

#if LOAD_NORMAL_DATA
					// normal data
					if (idx.normal_index >= 0) {
						float nx = tiny_attrib.normals[3 * idx.normal_index + 0];
						float ny = tiny_attrib.normals[3 * idx.normal_index + 1];
						float nz = tiny_attrib.normals[3 * idx.normal_index + 2];
						dst.normals[materialID].push_back(nx);
						dst.normals[materialID].push_back(ny);
						dst.normals[materialID].push_back(nz);
					}
#endif // LOAD_NORMAL_DATA

					//if (dst.indices[materialID].size() == 0) {
					//	dst.indices[materialID].push_back(0);
					//} else {
					//	GLuint lastIx = dst.indices[materialID].back();
					//	dst.indices[materialID].push_back(lastIx + 1);
					//}
				}
				index_offset += fv;
			}

			outPendingShapes.emplace_back(std::move(dst));

			LOG(LogDebug, "Shape has been parsed");
		}
	}

	Mesh* OBJLoader::craftMeshFrom(const string& shapeName) {
		for (size_t i = 0; i < t_shapes.size(); ++i) {
			if (t_shapes[i].name == shapeName) {
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

			for (auto materialID : shape.materialIDs) {
#if WARN_INVALID_FACE_MARTERIAL
				CHECK(materialID >= 0);
#endif
				auto& positions = shape.positions[materialID];
				auto& normals = shape.normals[materialID];
				auto& texcoords = shape.texcoords[materialID];
				auto& indices = shape.indices[materialID];

#if LOAD_NORMAL_DATA
				if (normals.size() < positions.size()) calculateNormal(t_attrib, indices, normals);
#else
				calculateNormal(positions, indices, normals);
#endif
				MeshGeometry* geom = new MeshGeometry;
				geom->setDrawArraysMode(false);
				geom->updatePositionData(&positions[0], static_cast<uint32>(positions.size()));
				geom->updateNormalData(&normals[0], static_cast<uint32>(normals.size()));
				if (texcoords.size() > 0) {
					geom->updateUVData(&texcoords[0], static_cast<uint32>(texcoords.size()));
				}
				geom->updateIndexData(&indices[0], static_cast<uint32>(indices.size()));
#if LOAD_AS_PBR_TEXTURE
				geom->calculateTangentBasis();
#endif
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
			switch (M->getMaterialID())
			{
#if LOAD_AS_PBR_TEXTURE
			case MATERIAL_ID::PBR_TEXTURE:
#else
			case MATERIAL_ID::FLAT_TEXTURE:
#endif
			{
				constexpr bool generateMipmap = true;
				constexpr bool sRGB = true;

				PendingTextures& pendingTextures = pendingTextureData[index];
				if (pendingTextures.glAlbedo == 0 && pendingTextures.albedo != nullptr) {
					pendingTextures.glAlbedo = pathos::createTextureFromBitmap(pendingTextures.albedo, generateMipmap, sRGB);
				}
				if (pendingTextures.glNormal == 0 && pendingTextures.normal != nullptr) {
					pendingTextures.glNormal = pathos::createTextureFromBitmap(pendingTextures.normal, generateMipmap, !sRGB);
				}
				if (pendingTextures.glRoughness == 0 && pendingTextures.roughness != nullptr) {
					pendingTextures.glRoughness = pathos::createTextureFromBitmap(pendingTextures.roughness, generateMipmap, !sRGB);
				}
				if (pendingTextures.glMetallic == 0 && pendingTextures.metallic != nullptr) {
					pendingTextures.glMetallic = pathos::createTextureFromBitmap(pendingTextures.metallic, generateMipmap, !sRGB);
				}
#if LOAD_AS_PBR_TEXTURE
				if (pendingTextures.glAlbedo != 0) static_cast<PBRTextureMaterial*>(M)->setAlbedo(pendingTextures.glAlbedo);
				if (pendingTextures.glNormal != 0) static_cast<PBRTextureMaterial*>(M)->setNormal(pendingTextures.glNormal);
				if (pendingTextures.glRoughness != 0) static_cast<PBRTextureMaterial*>(M)->setRoughness(pendingTextures.glRoughness);
				if (pendingTextures.glMetallic != 0) static_cast<PBRTextureMaterial*>(M)->setMetallic(pendingTextures.glMetallic);
#else
				if (pendingTextures.glAlbedo != 0) static_cast<TextureMaterial*>(M)->setTexture(pendingTextures.glAlbedo);
#endif
			}
				break;

			default:
				CHECKF(0, "Failed to process a pending material");
			}
		}

		return M;
	}

}
