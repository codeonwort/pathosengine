#include "pathos/loader/objloader.h"
#include "pathos/util/resource_finder.h"

#include <string>
#include <iostream>
#include <memory>
#include <set>
#include <map>

using namespace std;

//#define LOAD_NORMAL_DATA
//#define WARN_INVALID_FACE_MARTERIAL

namespace pathos {

	///////////////////////////////////////////////
	// OBJLoader

	/**
	* load a .obj file
	*/
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
		string err;
		tinyobj::LoadObj(&t_attrib, &t_shapes, &t_materials, &err, objFile.c_str(), mtlDir.c_str());

		std::cout << "Loading .obj file: \"" << objFile << "\"" << std::endl;
		if (!err.empty()) {
			std::cerr << "\tError while loading obj: " << err << std::endl;
			return false;
		}
		std::cout << "\tnumber of shapes: " << t_shapes.size() << std::endl;
		std::cout << "\tnumber of materials: " << t_materials.size() << std::endl;

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
		for (auto& bmp : bitmapDB) FreeImage_Unload(bmp.second);
		bitmapDB.clear();
		isPendingMaterial.clear();
		pendingTextureData.clear();
		textureDB.clear();
	}

	void OBJLoader::analyzeMaterials(const std::vector<tinyobj::material_t>& tiny_materials, std::vector<MeshMaterial*>& output) {
		static_cast<void>(tiny_materials);
		static_cast<void>(output);
		for (size_t i = 0; i < t_materials.size(); i++) {
			tinyobj::material_t& t_mat = t_materials[i];
			MeshMaterial* M = nullptr;

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
				pendingTextureData.insert(make_pair(i, bmp));
			} else {
				ColorMaterial* solidColor = new ColorMaterial;
				solidColor->setAmbient(t_mat.ambient[0], t_mat.ambient[1], t_mat.ambient[2]);
				solidColor->setDiffuse(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]);
				solidColor->setSpecular(t_mat.specular[0], t_mat.specular[1], t_mat.specular[2]);
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
		M->setAmbient(0.1f, 0.0f, 0.0f);
		M->setDiffuse(0.5f, 0.5f, 0.5f);
		M->setSpecular(1.0f, 1.0f, 1.0f);
		M->setAlpha(1.0f);
		materials.push_back(M);
	}

	void OBJLoader::reconstructShapes(const std::vector<tinyobj::shape_t>& tiny_shapes, const tinyobj::attrib_t& attrib, std::vector<PendingShape>& output) {
		output.clear();
		for (size_t i = 0; i < tiny_shapes.size(); ++i) {
			const tinyobj::shape_t& src = t_shapes[i];
			PendingShape dst;

			std::cout << "analyzing shape: " << src.name << std::endl;

			for (size_t f = 0; f < src.mesh.num_face_vertices.size(); ++f) {
				int faceMaterialID = src.mesh.material_ids[f];
#ifdef WARN_INVALID_FACE_MARTERIAL
				assert(faceMaterialID >= 0); // invalid material id
#endif
				dst.materialIDs.insert(faceMaterialID);
			}

			auto numMaterials = dst.materialIDs.size();
			size_t index_offset = 0;

			std::cout << "\tnumber of materials: " << numMaterials << std::endl;

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

			std::cout << "\tthis shape has been successfully parsed" << std::endl;
		}
	}

	Mesh* OBJLoader::craftMeshFrom(const string& shapeName) {
		for (size_t i = 0; i < t_shapes.size(); ++i) {
			if (t_shapes[i].name == shapeName) {
				return craftMeshFrom(i);
			}
		}
		return nullptr;
	}
	Mesh* OBJLoader::craftMeshFrom(unsigned int shapeIndex) { return craftMesh(shapeIndex, shapeIndex); }
	Mesh* OBJLoader::craftMeshFromAllShapes() { return craftMesh(0, pendingShapes.size() - 1); }

	Mesh* OBJLoader::craftMesh(unsigned int from, unsigned int to) {
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
				geom->updateVertexData(&positions[0], positions.size());
				geom->updateNormalData(&normals[0], normals.size());
				if (texcoords.size() > 0) geom->updateUVData(&texcoords[0], texcoords.size());
				geom->updateIndexData(&indices[0], indices.size());
				mesh->add(geom, getMaterial(materialID));
			}
		}

		return mesh;
	}

	MeshMaterial* OBJLoader::getMaterial(int index) {
		assert(-1 <= index && index < (int)materials.size());
		if (index == -1) return defaultMaterial;

		MeshMaterial* M = materials[index];

		if (isPendingMaterial[index]) {
			switch (M->getMaterialID()) {
			case MATERIAL_ID::FLAT_TEXTURE:
				GLuint texture;
				if (textureDB.find(index) == textureDB.end()) {
					texture = pathos::loadTexture(pendingTextureData[index]);
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

// template code for tinyobjloader
#if 0
std::string inputfile = "cornell_box.obj";
tinyobj::attrib_t attrib;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;

std::string err;
bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());

if (!err.empty()) { // `err` may contain warning message.
	std::cerr << err << std::endl;
}

if (!ret) {
	exit(1);
}

// Loop over shapes
for (size_t s = 0; s < shapes.size(); s++) {
	// Loop over faces(polygon)
	size_t index_offset = 0;
	for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
		int fv = shapes[s].mesh.num_face_vertices[f];

		// Loop over vertices in the face.
		for (size_t v = 0; v < fv; v++) {
			// access to vertex
			tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
			float vx = attrib.vertices[3 * idx.vertex_index + 0];
			float vy = attrib.vertices[3 * idx.vertex_index + 1];
			float vz = attrib.vertices[3 * idx.vertex_index + 2];
			float nx = attrib.normals[3 * idx.normal_index + 0];
			float ny = attrib.normals[3 * idx.normal_index + 1];
			float nz = attrib.normals[3 * idx.normal_index + 2];
			float tx = attrib.texcoords[2 * idx.texcoord_index + 0];
			float ty = attrib.texcoords[2 * idx.texcoord_index + 1];
		}
		index_offset += fv;

		// per-face material
		shapes[s].mesh.material_ids[f];
	}
}
#endif
