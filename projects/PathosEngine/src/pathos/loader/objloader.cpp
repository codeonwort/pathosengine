#include <pathos/loader/objloader.h>
#include <pathos/loader/imageloader.h>

#include <string>
#include <iostream>
#include <memory>
#include <set>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include "tiny_obj_loader.h"

using namespace std;

namespace pathos {

	///////////////////////////////////////////////
	// OBJLoader

	/**
	* load a .obj file
	*/
	OBJLoader::OBJLoader(const char* objFile, const char* mtlDir) :t_shapes(), t_materials(), geometries(), materialIndices(), materials() {
		//string err = tinyobj::LoadObj(t_shapes, t_materials, objFile, mtlDir);
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
			shared_ptr<MeshMaterial> mat = make_shared<MeshMaterial>();
			mat->setName(t_mat.name);
			if (t_mat.diffuse_texname.length() != 0) {
				string tex_path = mtlDir + t_mat.diffuse_texname;
				GLuint texID;
				cout << "required texture: " << tex_path << endl;
				auto it = textureDict.find(tex_path);
				if (it != textureDict.end()) {
					texID = it->second;
					cout << "texture for " << t_mat.diffuse_texname << " already exists. reuse it" << endl;
				}
				else {
					texID = loadTexture(loadImage(tex_path.c_str()));
					cout << "texture for " << t_mat.diffuse_texname << " does not exists. make one" << endl;
					textureDict.insert(pair<std::string, GLuint>(tex_path, texID));
				}
				mat->addPass(new TextureMaterialPass(texID));
			}
			else {
				ColorMaterialPass *pass = new ColorMaterialPass(t_mat.diffuse[0], t_mat.diffuse[1], t_mat.diffuse[2], 1.0f);
				pass->setAmbient(t_mat.ambient[0], t_mat.ambient[1], t_mat.ambient[2]);
				pass->setSpecular(t_mat.specular[0], t_mat.specular[1], t_mat.specular[2]);
				mat->addPass(pass);
			}
			materials.push_back(move(mat));
		}

		// if tehere is no material, create a random color material
		if (materials.size() == 0){
			shared_ptr<MeshMaterial> mat = make_shared<MeshMaterial>();
			ColorMaterialPass *pass = new ColorMaterialPass(0.5, 0.5, 0.5, 1.0f);
			pass->setAmbient(0, 0, 0);
			pass->setSpecular(1, 1, 1);
			//WireframeMaterialPass *pass = new WireframeMaterialPass(1, 1, 1, 1);
			mat->addPass(pass);
			materials.push_back(move(mat));
		}
		
		// convert each shape into geometries
		for (size_t i = 0; i < t_shapes.size(); i++) {
			tinyobj::shape_t &shape = t_shapes[i];
			size_t index_offset = 0;

			cout << "analyzing a shape: " << shape.name << endl;

			set<int> materialIDs; // material IDs used by faces of this shape
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++){
				int faceMatID = shape.mesh.material_ids[f];
				if (faceMatID < 0) faceMatID = 0;
				materialIDs.insert(faceMatID);
			}
			int numMaterialIDs = materialIDs.size();
			map<int, vector<GLuint>> indicesPerMaterial;

			cout << "-> num materials: " << numMaterialIDs << endl;

			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
				int fv = shape.mesh.num_face_vertices[f];
				int faceMatID = shape.mesh.material_ids[f];
				if (faceMatID < 0) faceMatID = 0;

				for (size_t v = 0; v < fv; v++){
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
					indicesPerMaterial[faceMatID].push_back(idx.vertex_index); // index buffer
				}
				index_offset += fv;
			}

			cout << "-> index buffers are filled" << endl;

			// Create a geometry for this shape
			MeshGeometry* originalGeom = new MeshGeometry;
			originalGeom->setName(shape.name);
			originalGeom->updateVertexData(&attrib.vertices[0], attrib.vertices.size());
			if (attrib.texcoords.size() > 0) originalGeom->updateUVData(&attrib.texcoords[0], attrib.texcoords.size());
			if (attrib.normals.size() > 0) originalGeom->updateNormalData(&attrib.normals[0], attrib.normals.size());
			//else geom->calculateNormals();
			cout << "-> geometry created for " << i << "-th material" << endl;

			// group the geometry with each index buffer
			for (auto it = materialIDs.begin(); it != materialIDs.end(); it++){
				int i = *it;
				MeshGeometry* geom = new MeshGeometry;
				geom->burrowVertexBuffer(originalGeom);
				//geom->burrowNormalBuffer(originalGeom);
				geom->burrowUVBuffer(originalGeom);
				geom->updateIndexData(&indicesPerMaterial[i][0], indicesPerMaterial[i].size());
				geom->calculateNormals();
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