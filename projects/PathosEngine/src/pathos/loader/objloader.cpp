#include "pathos/loader/objloader.h"
#include "pathos/loader/imageloader.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <string>
#include <iostream>
#include <memory>
#include <set>
#include <map>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include "tiny_obj_loader.h"

using namespace std;

namespace pathos {

	///////////////////////////////////////////////
	// OBJLoader

	/**
	* load a .obj file
	*/
	void calculateNormal(const tinyobj::attrib_t& attrib, const vector<GLuint>& indices, vector<GLfloat>& normals) {
		int numPos = attrib.vertices.size() / 3;
		glm::vec3* accum = new glm::vec3[numPos];
		unsigned int* counts = new unsigned int[numPos];
		for (int i = 0; i < numPos; i++){
			accum[i] = glm::vec3(0.0f);
			counts[i] = 0;
		}
		const auto& P = attrib.vertices;
		for (auto i = 0; i < indices.size(); i += 3){
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
		for (auto i = 0; i < numPos; i++){
			accum[i] = glm::normalize(accum[i]);
			//std::cerr << accum[i].x << ' ' << accum[i].y << ' ' << accum[i].z << endl;
		}
		for (auto i = 0; i < indices.size(); i++){
			normals.push_back(accum[indices[i]].x);
			normals.push_back(accum[indices[i]].y);
			normals.push_back(accum[indices[i]].z);
		}
		delete accum;
		delete counts;
	}

	OBJLoader::OBJLoader(const char* objFile, const char* mtlDir) :t_shapes(), t_materials(), geometries(), materialIndices(), materials() {
		string err;
		tinyobj::attrib_t attrib;
		bool ret = tinyobj::LoadObj(&attrib, &t_shapes, &t_materials, &err, objFile, mtlDir);

		cout << "Loading .obj file: \"" << objFile << "\"" << endl;
		if (!err.empty()) {
			cerr << err << endl;
		}
		cout << "  number of shapes: " << t_shapes.size() << endl;
		cout << "  number of materials: " << t_materials.size() << endl;
		
		// gather materials
		map<string, GLuint> textureDict;
		for (size_t i = 0; i < t_materials.size(); i++) {
			tinyobj::material_t &t_mat = t_materials[i];
			MeshMaterial* mat = nullptr;
			
			// currently, only plain texture or color materials are supported
			if (t_mat.diffuse_texname.length() != 0) {
				string tex_path = mtlDir + t_mat.diffuse_texname;
				GLuint texID;
				cout << "required texture: " << tex_path << endl;
				auto it = textureDict.find(tex_path);
				if (it != textureDict.end()) {
					texID = it->second;
					cout << "texture for " << t_mat.diffuse_texname << " already exists. reuse it" << endl;
				} else {
					texID = loadTexture(loadImage(tex_path.c_str()));
					cout << "texture for " << t_mat.diffuse_texname << " does not exists. make one" << endl;
					textureDict.insert(pair<std::string, GLuint>(tex_path, texID));
				}
				if (t_mat.bump_texname.length() != 0){
					// TODO: support bump mapping
					cout << "!!!! BUMP TEXTURE HERE: " << t_mat.bump_texname << endl;
				}
				TextureMaterial* flatTexture = new TextureMaterial(texID);
				mat = flatTexture;
			} else {
				ColorMaterial* solidColor = new ColorMaterial;
				solidColor->setAmbient(t_mat.ambient[0], t_mat.ambient[1], t_mat.ambient[2]);
				solidColor->setDiffuse(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2]);
				solidColor->setSpecular(t_mat.specular[0], t_mat.specular[1], t_mat.specular[2]);
				solidColor->setAlpha(1.0f);
				mat = solidColor;
			}

			mat->setName(t_mat.name);
			materials.push_back(mat);
		}

		// if there is no material, create a random color material
		if (materials.size() == 0){
			ColorMaterial *defaultMaterial = new ColorMaterial;
			defaultMaterial->setDiffuse(0.5f, 0.5f, 0.5f);
			//WireframeMaterialPass *pass = new WireframeMaterialPass(1, 1, 1, 1);
			MeshMaterial* mat = defaultMaterial;
			materials.push_back(move(mat));
		}
		
		// convert each shape into geometries
		for (size_t i = 0; i < t_shapes.size(); i++) {
			tinyobj::shape_t &shape = t_shapes[i];
			
			cout << "analyzing a shape: " << shape.name << endl;

			set<int> materialIDs; // material IDs used by faces of this shape
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++){
				int faceMatID = shape.mesh.material_ids[f];
				if (faceMatID < 0) faceMatID = 0;
				materialIDs.insert(faceMatID);
			}
			int numMaterialIDs = materialIDs.size();

			map<int, vector<GLfloat>> positionMap, normalMap, texcoordMap;
			map<int, vector<GLuint>> indexMap;

			cout << "-> num materials: " << numMaterialIDs << endl;

			size_t index_offset = 0;
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
				int fv = shape.mesh.num_face_vertices[f];
				int faceMatID = shape.mesh.material_ids[f];
				if (faceMatID < 0) faceMatID = 0;

				for (size_t v = 0; v < fv; v++){
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
					float vx = attrib.vertices[3 * idx.vertex_index + 0];
					float vy = attrib.vertices[3 * idx.vertex_index + 1];
					float vz = attrib.vertices[3 * idx.vertex_index + 2];
					positionMap[faceMatID].push_back(vx);
					positionMap[faceMatID].push_back(vy);
					positionMap[faceMatID].push_back(vz);
					/*if (idx.normal_index >= 0){
						float nx = attrib.normals[3 * idx.normal_index + 0];
						float ny = attrib.normals[3 * idx.normal_index + 1];
						float nz = attrib.normals[3 * idx.normal_index + 2];
						normalMap[faceMatID].push_back(nx);
						normalMap[faceMatID].push_back(ny);
						normalMap[faceMatID].push_back(nz);
					}*/
					if (idx.texcoord_index >= 0){
						float u = attrib.texcoords[2 * idx.texcoord_index + 0];
						float v = attrib.texcoords[2 * idx.texcoord_index + 1];
						texcoordMap[faceMatID].push_back(u);
						texcoordMap[faceMatID].push_back(v);
					}
					indexMap[faceMatID].push_back(idx.vertex_index);
				}
				index_offset += fv;
			}

			cout << "-> buffers are filled" << endl;

			// group the geometry with each index buffer
			for (auto it = materialIDs.begin(); it != materialIDs.end(); it++){
				int i = *it;
				MeshGeometry* geom = new MeshGeometry;
				geom->setDrawArraysMode(true);
				geom->updateVertexData(&positionMap[i][0], positionMap[i].size());
				geom->updateIndexData(&indexMap[i][0], indexMap[i].size());
				//if(normalMap[i].size() > 0) geom->updateNormalData(&normalMap[i][0], normalMap[i].size());
				//else{
					calculateNormal(attrib, indexMap[i], normalMap[i]);
					std::cout << "-> auto calculating normals..." << endl;
					//geom->calculateNormals();
					geom->updateNormalData(&normalMap[i][0], normalMap[i].size());
				//}
				if(texcoordMap[i].size() > 0) geom->updateUVData(&texcoordMap[i][0], texcoordMap[i].size());
				geometries.push_back(geom);
				materialIndices.push_back(i);
			}
		}
	}

	Mesh* OBJLoader::craftMesh(size_t start, size_t end, string name) {
		Mesh* m = new Mesh(nullptr, nullptr);
		m->setName(name);
		for (auto i = start; i < end; i++) {
			m->add(geometries[i], materials[materialIndices[i]]);
		}
		return m;
	}

	///////////////////////////////////////////////
	// ColladaLoader
	ColladaLoader::ColladaLoader(const char* file):geometries() {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
		if (!scene){
			std::cerr << "error loading collada: " << *file << std::endl;
		}

		std::cout << "num mesh: " << scene->mNumMeshes << std::endl;
		for (unsigned int i = 0; i < scene->mNumMeshes; i++){
			buildGeometry(scene->mMeshes[i]);
		}
	}
	void ColladaLoader::buildGeometry(aiMesh* mesh) {
		const int vertexTotalSize = sizeof(aiVector3D) * 2 + sizeof(aiVector2D);
		int totalVertices = 0;

		MeshGeometry* G = new MeshGeometry;
		GLfloat* positionData = new GLfloat[mesh->mNumVertices * 3];
		GLfloat* normalData = new GLfloat[mesh->mNumVertices * 3];
		GLfloat* uvData = new GLfloat[mesh->mNumVertices * 2];
		// copy data
		if (mesh->HasPositions()) for (unsigned int i = 0; i < mesh->mNumVertices; i++){
			aiVector3D& pos = mesh->mVertices[i];
			positionData[i * 3] = pos.x; positionData[i * 3 + 1] = pos.y; positionData[i * 3 + 2] = pos.z;
		}
		if (mesh->HasNormals()) for (unsigned int i = 0; i < mesh->mNumVertices; i++){
			aiVector3D& norm = mesh->mNormals[i];
			normalData[i * 3] = norm.x; normalData[i * 3 + 1] = norm.y; normalData[i * 3 + 2] = norm.z;
		}
		if (mesh->HasTextureCoords(0)) for (unsigned int i = 0; i < mesh->mNumVertices; i++){
			aiVector3D& uv = mesh->mTextureCoords[0][i];
			uvData[i * 2] = uv.x; uvData[i * 2 + 1] = uv.y;
		}
		GLuint* indexData = new GLuint[mesh->mNumFaces * 3];
		for (unsigned int i = 0; i < mesh->mNumFaces; i++){
			const aiFace& face = mesh->mFaces[i];
			for (int k = 0; k < 3; k++) indexData[i * 3 + k] = face.mIndices[k];
		}
		// upload to GPU memory
		if(mesh->HasPositions()) G->updateVertexData(positionData, mesh->mNumVertices);
		if(mesh->HasNormals()) G->updateNormalData(normalData, mesh->mNumVertices);
		else G->calculateNormals();
		if(mesh->HasTextureCoords(0)) G->updateUVData(uvData, mesh->mNumVertices);
		G->updateIndexData(indexData, mesh->mNumFaces * 3);
		// add this geometry to list
		geometries.push_back(G);
	}

}

/* template code for tinyobjloader
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
float vx = attrib.vertices[3*idx.vertex_index+0];
float vy = attrib.vertices[3*idx.vertex_index+1];
float vz = attrib.vertices[3*idx.vertex_index+2];
float nx = attrib.normals[3*idx.normal_index+0];
float ny = attrib.normals[3*idx.normal_index+1];
float nz = attrib.normals[3*idx.normal_index+2];
float tx = attrib.texcoords[2*idx.texcoord_index+0];
float ty = attrib.texcoords[2*idx.texcoord_index+1];
}
index_offset += fv;

// per-face material
shapes[s].mesh.material_ids[f];
}
}
*/