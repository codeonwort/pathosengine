#include "pathos/engine.h"
#include "pathos/loader/objloader.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#define LOAD_NORMAL_DATA            0
#define WARN_INVALID_FACE_MARTERIAL 0
#define LOAD_AS_PBR_TEXTURE         1

static constexpr float TRANSLUCENT_THRESHOLD = 0.999f;

namespace pathos {

	void calculateNormal(const std::vector<GLfloat>& positions, const std::vector<GLuint>& indices, std::vector<GLfloat>& outNormals) {
		outNormals.clear();

		uint32 numPos = (uint32)(positions.size() / 3);
		std::vector<glm::vec3> accum(numPos, glm::vec3(0.0f));
		std::vector<uint32> counts(numPos, 0);

		const std::vector<float>& P = positions;

		for (size_t i = 0u; i < indices.size(); i += 3) {
			GLuint i0 = indices[i + 0], i1 = indices[i + 1], i2 = indices[i + 2];
			GLuint p0 = i0 * 3, p1 = i1 * 3, p2 = i2 * 3;
			glm::vec3 a = glm::vec3(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			glm::vec3 b = glm::vec3(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);
			if (a == b) {
				continue;
			}
			//auto norm = glm::normalize(glm::cross(a, b));
			glm::vec3 norm = glm::cross(a, b);

			accum[i0] *= counts[i0]; accum[i1] *= counts[i1]; accum[i2] *= counts[i2];
			accum[i0] += norm; accum[i1] += norm; accum[i2] += norm;
			counts[i0] ++; counts[i1] ++; counts[i2] ++;
			accum[i0] /= counts[i0]; accum[i1] /= counts[i1]; accum[i2] /= counts[i2];
		}
		for (uint32 i = 0u; i < numPos; i++) {
			accum[i] = glm::normalize(accum[i]);
		}
		for (uint32 i = 0u; i < indices.size(); i++) {
			outNormals.push_back(accum[indices[i]].x);
			outNormals.push_back(accum[indices[i]].y);
			outNormals.push_back(accum[indices[i]].z);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////

	OBJLoader::OBJLoader() {}

	OBJLoader::OBJLoader(const char* objFile, const char* mtlDir) {
		bool ok = load(objFile, mtlDir);
		CHECK(ok);
	}

	bool OBJLoader::load(const char* _objFile, const char* _mtlDir) {
		std::string objFile = ResourceFinder::get().find(_objFile);
		mtlDir = ResourceFinder::get().find(_mtlDir);

		// read data using tinyobjloader
		std::string err;
		bool loaded = tinyobj::LoadObj(&t_attrib, &t_shapes, &t_materials, &err, objFile.c_str(), mtlDir.c_str());

		LOG(LogInfo, "Loading .obj file: %s", objFile.data());
		if (!err.empty()) {
			LOG(LogError, "Error while loading OBJ file: %s", err.data());
			if (!loaded) {
				return false;
			}
		}
		LOG(LogInfo, "Number of shapes: %d", (int32)t_shapes.size());
		LOG(LogInfo, "Number of materials: %d", (int32)t_materials.size());

		// reconstruct data
		analyzeMaterials(t_materials, materials);
		reconstructShapes(t_shapes, t_attrib, pendingShapes);

		return true;
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
		for (auto& bmp : bitmapDB) {
			FreeImage_Unload(bmp.second);
		}
		bitmapDB.clear();
		isPendingMaterial.clear();
		pendingTextureData.clear();
		textureDB.clear();
	}

	void OBJLoader::analyzeMaterials(const std::vector<tinyobj::material_t>& tiny_materials, std::vector<Material*>& output) {
		for (size_t i = 0; i < t_materials.size(); i++) {
			tinyobj::material_t& t_mat = t_materials[i];
			Material* M = nullptr;

			if (t_mat.diffuse_texname.length() > 0)
			{
				std::string image_path = mtlDir + t_mat.diffuse_texname;
				FIBITMAP* bmp;
				if (bitmapDB.find(image_path) == bitmapDB.end()) {
					bmp = loadImage(image_path.c_str());
					bitmapDB.insert(std::make_pair(image_path, bmp));
				} else {
					bmp = bitmapDB[image_path];
				}

				// pending request
#if LOAD_AS_PBR_TEXTURE
				M = new PBRTextureMaterial(
					0,
					gEngine->getSystemTexture2DBlue(),   // #todo-loader: Parse normal texture
					gEngine->getSystemTexture2DGrey(),   // #todo-loader: Parse metallic texture
					gEngine->getSystemTexture2DBlack(),  // #todo-loader: Parse roughness texture
					gEngine->getSystemTexture2DWhite()); // #todo-loader: Parse AO texture
#else
				M = new TextureMaterial(0);
#endif

				isPendingMaterial.push_back(true);
				pendingTextureData.insert(make_pair(static_cast<int32>(i), PendingTexture(bmp, true)));
			}
			else if (t_mat.dissolve < TRANSLUCENT_THRESHOLD
				|| t_mat.transmittance[0] < TRANSLUCENT_THRESHOLD
				|| t_mat.transmittance[1] < TRANSLUCENT_THRESHOLD
				|| t_mat.transmittance[2] < TRANSLUCENT_THRESHOLD)
			{
				TranslucentColorMaterial* translucentColor = new TranslucentColorMaterial;
				translucentColor->setAlbedo(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]);
				translucentColor->setMetallic(t_mat.metallic);
				translucentColor->setRoughness(t_mat.roughness);
				translucentColor->setOpacity(t_mat.dissolve);
				
				M = translucentColor;
				isPendingMaterial.push_back(false);
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
				// #todo-loader: Parse emission

				M = solidColor;
				isPendingMaterial.push_back(false);
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
			PendingShape dst;

			LOG(LogDebug, "Analyzing OBJ shape[%d]: %s", i, src.name.data());

			for (size_t f = 0; f < src.mesh.num_face_vertices.size(); ++f) {
				int32 faceMaterialID = (int32)(src.mesh.material_ids[f]);
#if WARN_INVALID_FACE_MARTERIAL
				CHECK(faceMaterialID >= 0); // invalid material id
#endif
				dst.materialIDs.insert(faceMaterialID);
			}

			size_t numMaterials = dst.materialIDs.size();
			size_t index_offset = 0;

			LOG(LogDebug, "Number of materials for this shape: %d", numMaterials);

			for (size_t f = 0u; f < src.mesh.num_face_vertices.size(); ++f) {
				int32 fv = src.mesh.num_face_vertices[f];
				CHECK(fv == 3);
				int32 materialID = src.mesh.material_ids[f];
				for (int32 v = 0; v < fv; ++v) {
					tinyobj::index_t idx = src.mesh.indices[index_offset + v];
					// position data
					float vx = tiny_attrib.vertices[3 * idx.vertex_index + 0];
					float vy = tiny_attrib.vertices[3 * idx.vertex_index + 1];
					float vz = tiny_attrib.vertices[3 * idx.vertex_index + 2];
					dst.positions[materialID].push_back(vx);
					dst.positions[materialID].push_back(vy);
					dst.positions[materialID].push_back(vz);
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
					// texcoord data
					if (idx.texcoord_index >= 0) {
						float u = tiny_attrib.texcoords[2 * idx.texcoord_index + 0];
						float v = tiny_attrib.texcoords[2 * idx.texcoord_index + 1];
						dst.texcoords[materialID].push_back(u);
						dst.texcoords[materialID].push_back(v);
					}

					//dst.indices[materialID].push_back(idx.vertex_index);
					if (dst.indices[materialID].size() == 0) {
						dst.indices[materialID].push_back(0);
					} else {
						GLuint lastIx = dst.indices[materialID].back();
						dst.indices[materialID].push_back(lastIx + 1);
					}
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
				geom->setDrawArraysMode(true);
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

		if (isPendingMaterial[index]) {
			switch (M->getMaterialID())
			{
#if LOAD_AS_PBR_TEXTURE
			case MATERIAL_ID::PBR_TEXTURE:
#else
			case MATERIAL_ID::FLAT_TEXTURE:
#endif
				GLuint texture;
				if (textureDB.find(index) == textureDB.end()) {
					constexpr bool generateMipmap = true;
					const PendingTexture& pendingTexture = pendingTextureData[index];
					texture = pathos::loadTexture(pendingTexture.rawData, generateMipmap, pendingTexture.sRGB);
					textureDB.insert(make_pair(index, texture));
				} else {
					texture = textureDB[index];
				}
#if LOAD_AS_PBR_TEXTURE
				static_cast<PBRTextureMaterial*>(M)->setAlbedo(texture);
#else
				static_cast<TextureMaterial*>(M)->setTexture(texture);
#endif
				break;

			default:
				// no impl for a pending material. find out what's missing!
				CHECK(0);
			}
		}

		return M;
	}

}
