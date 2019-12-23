#include "pathos/loader/objloader.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include <string>
#include <memory>

//#define LOAD_NORMAL_DATA
//#define WARN_INVALID_FACE_MARTERIAL

namespace pathos {

	void calculateNormal(const tinyobj::attrib_t& attrib, const vector<GLuint>& indices, vector<GLfloat>& normals) {
		normals.clear();
		auto numPos = attrib.vertices.size() / 3;
		glm::vec3* accum = new glm::vec3[numPos];
		unsigned int* counts = new unsigned int[numPos];
		for (auto i = 0u; i < numPos; i++) {
			accum[i] = glm::vec3(0.0f);
			counts[i] = 0;
		}
		const auto& P = attrib.vertices;
		for (auto i = 0u; i < indices.size(); i += 3) {
			auto i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
			auto p0 = i0 * 3, p1 = i1 * 3, p2 = i2 * 3;
			glm::vec3 a = glm::vec3(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			glm::vec3 b = glm::vec3(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);
			if (a == b) continue;
			//auto norm = glm::normalize(glm::cross(a, b));
			auto norm = glm::cross(a, b);

			accum[i0] *= counts[i0]; accum[i1] *= counts[i1]; accum[i2] *= counts[i2];
			accum[i0] += norm; accum[i1] += norm; accum[i2] += norm;
			counts[i0] ++; counts[i1] ++; counts[i2] ++;
			accum[i0] /= counts[i0]; accum[i1] /= counts[i1]; accum[i2] /= counts[i2];
		}
		for (auto i = 0u; i < numPos; i++) {
			accum[i] = glm::normalize(accum[i]);
		}
		for (auto i = 0u; i < indices.size(); i++) {
			normals.push_back(accum[indices[i]].x);
			normals.push_back(accum[indices[i]].y);
			normals.push_back(accum[indices[i]].z);
		}
		delete accum;
		delete counts;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////

	OBJLoader::OBJLoader() {}

	OBJLoader::OBJLoader(const char* objFile, const char* mtlDir) {
		bool ok = load(objFile, mtlDir);
		assert(ok);
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
		LOG(LogInfo, "Number of shapes: %d", (int32_t)t_shapes.size());
		LOG(LogInfo, "Number of materials: %d", (int32_t)t_materials.size());

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
		static_cast<void>(tiny_materials);
		static_cast<void>(output);

		for (size_t i = 0; i < t_materials.size(); i++) {
			tinyobj::material_t& t_mat = t_materials[i];
			Material* M = nullptr;

			if (t_mat.diffuse_texname.length() > 0) {
				std::string image_path = mtlDir + t_mat.diffuse_texname;
				FIBITMAP* bmp;
				if (bitmapDB.find(image_path) == bitmapDB.end()) {
					bmp = loadImage(image_path.c_str());
					bitmapDB.insert(std::make_pair(image_path, bmp));
				} else {
					bmp = bitmapDB[image_path];
				}
				M = new TextureMaterial(0); // pending request
				isPendingMaterial.push_back(true);
				pendingTextureData.insert(make_pair(static_cast<int32_t>(i), bmp));
			} else {
				ColorMaterial* solidColor = new ColorMaterial;
// 				solidColor->setAmbient(t_mat.ambient[0], t_mat.ambient[1], t_mat.ambient[2]);
// 				solidColor->setSpecular(t_mat.specular[0], t_mat.specular[1], t_mat.specular[2]);
				solidColor->setAlbedo(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]);
				solidColor->setMetallic(t_mat.metallic);
				solidColor->setRoughness(t_mat.roughness);
				solidColor->setAlpha(1.0f);
				M = solidColor;
				isPendingMaterial.push_back(false);
			}

			M->setName(t_mat.name);
			materials.push_back(M);
		}

		// used for shapes whose material id is invalid
		defaultMaterial = new ColorMaterial;
		ColorMaterial* M = defaultMaterial;
		M->setAlbedo(0.5f, 0.5f, 0.5f);
		M->setAlpha(1.0f);
		materials.push_back(M);
	}

	void OBJLoader::reconstructShapes(const std::vector<tinyobj::shape_t>& tiny_shapes, const tinyobj::attrib_t& attrib, std::vector<PendingShape>& output) {
		output.clear();
		for (size_t i = 0; i < tiny_shapes.size(); ++i) {
			const tinyobj::shape_t& src = t_shapes[i];
			PendingShape dst;

			LOG(LogDebug, "Analyzing OBJ shape[%d]: %s", i, src.name.data());

			for (size_t f = 0; f < src.mesh.num_face_vertices.size(); ++f) {
				int faceMaterialID = src.mesh.material_ids[f];
#ifdef WARN_INVALID_FACE_MARTERIAL
				assert(faceMaterialID >= 0); // invalid material id
#endif
				dst.materialIDs.insert(faceMaterialID);
			}

			auto numMaterials = dst.materialIDs.size();
			size_t index_offset = 0;

			LOG(LogDebug, "Number of materials for this shape: %d", numMaterials);

			for (auto f = 0u; f < src.mesh.num_face_vertices.size(); ++f) {
				int fv = src.mesh.num_face_vertices[f];
				int materialID = src.mesh.material_ids[f];
				for (auto v = 0; v < fv; ++v) {
					tinyobj::index_t idx = src.mesh.indices[index_offset + v];
					// position data
					float vx = attrib.vertices[3 * idx.vertex_index + 0];
					float vy = attrib.vertices[3 * idx.vertex_index + 1];
					float vz = attrib.vertices[3 * idx.vertex_index + 2];
					dst.positions[materialID].push_back(vx);
					dst.positions[materialID].push_back(vy);
					dst.positions[materialID].push_back(vz);
#ifdef LOAD_NORMAL_DATA
					// normal data
					if (idx.normal_index >= 0) {
						float nx = attrib.normals[3 * idx.normal_index + 0];
						float ny = attrib.normals[3 * idx.normal_index + 1];
						float nz = attrib.normals[3 * idx.normal_index + 2];
						dst.normals[materialID].push_back(nx);
						dst.normals[materialID].push_back(ny);
						dst.normals[materialID].push_back(nz);
					}
#endif // LOAD_NORMAL_DATA
					// texcoord data
					if (idx.texcoord_index >= 0) {
						float u = attrib.texcoords[2 * idx.texcoord_index + 0];
						float v = attrib.texcoords[2 * idx.texcoord_index + 1];
						dst.texcoords[materialID].push_back(u);
						dst.texcoords[materialID].push_back(v);
					}
					dst.indices[materialID].push_back(idx.vertex_index);
				}
				index_offset += fv;
			}

			output.emplace_back(std::move(dst));

			LOG(LogDebug, "Shape has been parsed");
		}
	}

	Mesh* OBJLoader::craftMeshFrom(const string& shapeName) {
		for (size_t i = 0; i < t_shapes.size(); ++i) {
			if (t_shapes[i].name == shapeName) {
				return craftMeshFrom(static_cast<uint32_t>(i));
			}
		}
		return nullptr;
	}
	Mesh* OBJLoader::craftMeshFrom(uint32_t shapeIndex) {
		return craftMesh(shapeIndex, shapeIndex);
	}
	Mesh* OBJLoader::craftMeshFromAllShapes() {
		return craftMesh(0, static_cast<uint32_t>(pendingShapes.size() - 1));
	}

	Mesh* OBJLoader::craftMesh(uint32_t from, uint32_t to) {
		assert(0 <= from && from < pendingShapes.size());
		assert(0 <= to && to < pendingShapes.size());
		assert(from <= to);

		Mesh* mesh = new Mesh;

		for (unsigned int i = from; i <= to; ++i) {
			PendingShape& shape = pendingShapes[i];

			for (auto materialID : shape.materialIDs) {
#ifdef WARN_INVALID_FACE_MARTERIAL
				assert(materialID >= 0);
#endif
				auto& positions = shape.positions[materialID];
				auto& normals = shape.normals[materialID];
				auto& texcoords = shape.texcoords[materialID];
				auto& indices = shape.indices[materialID];

#ifdef LOAD_NORMAL_DATA
				if (normals.size() < positions.size()) calculateNormal(t_attrib, indices, normals);
#else
				calculateNormal(t_attrib, indices, normals);
#endif
				MeshGeometry* geom = new MeshGeometry;
				geom->setDrawArraysMode(true);
				geom->updatePositionData(&positions[0], static_cast<uint32_t>(positions.size()));
				geom->updateNormalData(&normals[0], static_cast<uint32>(normals.size()));
				if (texcoords.size() > 0) {
					geom->updateUVData(&texcoords[0], static_cast<uint32_t>(texcoords.size()));
				}
				geom->updateIndexData(&indices[0], static_cast<uint32_t>(indices.size()));
				mesh->add(geom, getMaterial(materialID));
			}
		}

		return mesh;
	}

	Material* OBJLoader::getMaterial(int32_t index) {
		assert(-1 <= index && index < (int32_t)materials.size());
		if (index == -1) {
			return defaultMaterial;
		}

		Material* M = materials[index];

		if (isPendingMaterial[index]) {
			switch (M->getMaterialID()) {
			case MATERIAL_ID::FLAT_TEXTURE:
				GLuint texture;
				if (textureDB.find(index) == textureDB.end()) {
					// #todo: set sRGB=true only for diffuse texture
					constexpr bool sRGB = true;
					texture = pathos::loadTexture(pendingTextureData[index], true, sRGB);
					textureDB.insert(make_pair(index, texture));
				} else {
					texture = textureDB[index];
				}
				static_cast<TextureMaterial*>(M)->setTexture(texture);
				break;
			default:
				// no impl for a pending material. find out what's missing!
				assert(0);
			}
		}

		return M;
	}

}
