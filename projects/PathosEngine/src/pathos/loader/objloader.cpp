#include <pathos/loader/objloader.h>
#include <pathos/loader/imageloader.h>

#include <string>
#include <iostream>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

namespace pathos {

	///////////////////////////////////////////////
	// OBJLoader

	/**
	* load a .obj file
	*/
	OBJLoader::OBJLoader(const char* objFile, const char* mtlDir) :t_shapes(), t_materials(), geometries(), materialIndices(), materials() {
		string err = tinyobj::LoadObj(t_shapes, t_materials, objFile, mtlDir);
		cout << "Loading .obj file: \"" << objFile << "\"" << endl;
		if (!err.empty()) {
			cerr << err << endl;
			return;
		}
		cout << "  number of shapes: " << t_shapes.size() << endl;
		cout << "  number of materials: " << t_materials.size() << endl;
		
		// create materials
		map<string, GLuint> textureDict;
		for (size_t i = 0; i < t_materials.size(); i++) {
			tinyobj::material_t &t_mat = t_materials[i];
			shared_ptr<MeshMaterial> mat = make_shared<MeshMaterial>();
			mat->setName(t_mat.name);
			if (t_mat.diffuse_texname.length() != 0) {
				string tex_path = "../resources/models/" + t_mat.diffuse_texname;
				GLuint texID;
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
		
		// wrap shapes with geometries
		for (size_t i = 0; i < t_shapes.size(); i++) {
			tinyobj::shape_t &shape = t_shapes[i];
			MeshGeometry* geom = new MeshGeometry;
			geom->updateVertexData(&shape.mesh.positions[0], shape.mesh.positions.size());
			if (shape.mesh.texcoords.size() > 0){
				geom->updateUVData(&shape.mesh.texcoords[0], shape.mesh.texcoords.size());
			}
			geom->updateNormalData(&shape.mesh.normals[0], shape.mesh.normals.size());
			geom->updateIndexData(&shape.mesh.indices[0], shape.mesh.indices.size());
			geom->setName(shape.name);
			geometries.push_back(geom);
			materialIndices.push_back(shape.mesh.material_ids[0]);
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

/*for (size_t j = 0; j < materials.size(); j++) {
tinyobj::material_t mat = materials[j];
cout << "=====================" << endl;
cout << "name: " << mat.name << endl;
cout << "ambient: " << mat.ambient[0] << " " << mat.ambient[1] << " " << mat.ambient[2] << endl;
cout << "diffuse: " << mat.diffuse[0] << " " << mat.diffuse[1] << " " << mat.diffuse[2] << endl;
cout << "specular: " << mat.specular[0] << " " << mat.specular[1] << " " << mat.specular[2] << endl;
cout << "transmittance: " << mat.transmittance[0] << " " << mat.transmittance[1] << " " << mat.transmittance[2] << endl;
cout << "emission: " << mat.emission[0] << " " << mat.emission[1] << " " << mat.emission[2] << endl;
cout << "shiniess: " << mat.shininess << endl;
cout << "index of refraction: " << mat.ior << endl;
cout << "dissolve: " << mat.dissolve << endl;
cout << "illumination model: " << mat.illum << endl;
cout << "ambient_texname: " << mat.ambient_texname << endl;
cout << "diffuse_texname: " << mat.diffuse_texname << endl;
cout << "specular_texname: " << mat.specular_texname << endl;
cout << "specular_highlight_texname: " << mat.specular_highlight_texname << endl;
cout << "bump_texname: " << mat.bump_texname << endl;
cout << "displacement_texname: " << mat.displacement_texname << endl;
cout << "alpha_texname: " << mat.alpha_texname << endl;
cout << "=====================" << endl;
}*/