#include "daeloader.h"

#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"

#if defined(_DEBUG)
#include <iostream>
using namespace std;
#endif

namespace pathos {

	//////////////////////////////////////////////////////////////////////
	// public interface

	DAELoader::DAELoader() {}

	DAELoader::DAELoader(const char* filename, unsigned int flags) {
		load(filename, flags);
	}

	DAELoader::~DAELoader() {
		unload();
	}

	bool DAELoader::load(const char* filename, unsigned int flags) {
		if (scene) abort();

		scene = aiImportFile(filename, flags);
		if (!scene) return false;

		// mesh
		loadMeshes();

		// animation
		auto anim = scene->mAnimations[0];
		auto chans = anim->mChannels;
		for (int i = 0; i < anim->mNumChannels; ++i) {
			auto chan = anim->mChannels[i];
			int z = 0;
		}
	}

	bool DAELoader::unload() {
		if (!scene) return false;

		scene = nullptr;
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// sub functions
	
	void DAELoader::loadMeshes() {
		for (auto i = 0; i < scene->mNumMeshes; ++i) {
			const auto mesh = scene->mMeshes[i];

			// at least positions and indices are needed
			assert(mesh->HasPositions());
			assert(mesh->HasFaces());

			GLfloat* vertices = reinterpret_cast<GLfloat*>(&mesh->mVertices[0]);
			GLfloat* normals = reinterpret_cast<GLfloat*>(&mesh->mNormals[0]);
			GLfloat* texcoords = reinterpret_cast<GLfloat*>(&mesh->mTextureCoords[0]);
			GLfloat* tangents = reinterpret_cast<GLfloat*>(&mesh->mTangents[0]);
			GLfloat* bitangents = reinterpret_cast<GLfloat*>(&mesh->mBitangents[0]);

			std::vector<GLuint> indices;
			for (auto i = 0; i < mesh->mNumFaces; ++i) {
				const auto& face = mesh->mFaces[i];
				assert(face.mNumIndices == 3); // TODO: support other primitives
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}
			
			MeshGeometry* G = new MeshGeometry;
			if (mesh->HasPositions()) G->updateVertexData(vertices, 3 * mesh->mNumVertices);
			if (mesh->HasNormals()) G->updateNormalData(normals, 3 * mesh->mNumVertices);
			if (mesh->HasTextureCoords(0)) G->updateUVData(texcoords, 2 * mesh->mNumVertices);
			if (mesh->HasTangentsAndBitangents()) {
				G->updateTangentData(tangents, 3 * mesh->mNumVertices);
				G->updateBitangentData(bitangents, 3 * mesh->mNumVertices);
			}
			G->updateIndexData(&indices[0], indices.size());

			ColorMaterial* M = new ColorMaterial;
			M->setAmbient(1.0f, 0.0f, 0.0f);
			meshes.push_back(new Mesh(G, M));
		}
	}


}