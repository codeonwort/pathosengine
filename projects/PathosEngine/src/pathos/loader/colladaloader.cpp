#include "colladaloader.h"

#ifndef DEPRECATED

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <iostream>

namespace pathos {

	ColladaLoader::ColladaLoader(const char* file) :geometries() {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
		if (!scene) {
			std::cerr << "error loading collada: " << *file << std::endl;
		}

		std::cout << "num mesh: " << scene->mNumMeshes << std::endl;
		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
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
		if (mesh->HasPositions()) for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D& pos = mesh->mVertices[i];
			positionData[i * 3] = pos.x; positionData[i * 3 + 1] = pos.y; positionData[i * 3 + 2] = pos.z;
		}
		if (mesh->HasNormals()) for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D& norm = mesh->mNormals[i];
			normalData[i * 3] = norm.x; normalData[i * 3 + 1] = norm.y; normalData[i * 3 + 2] = norm.z;
		}
		if (mesh->HasTextureCoords(0)) for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D& uv = mesh->mTextureCoords[0][i];
			uvData[i * 2] = uv.x; uvData[i * 2 + 1] = uv.y;
		}
		GLuint* indexData = new GLuint[mesh->mNumFaces * 3];
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];
			for (int k = 0; k < 3; k++) indexData[i * 3 + k] = face.mIndices[k];
		}
		// upload to GPU memory
		if (mesh->HasPositions()) G->updateVertexData(positionData, mesh->mNumVertices);
		if (mesh->HasNormals()) G->updateNormalData(normalData, mesh->mNumVertices);
		else G->calculateNormals();
		if (mesh->HasTextureCoords(0)) G->updateUVData(uvData, mesh->mNumVertices);
		G->updateIndexData(indexData, mesh->mNumFaces * 3);
		// add this geometry to list
		geometries.push_back(G);
	}

}

#endif