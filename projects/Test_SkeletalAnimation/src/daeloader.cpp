#include "daeloader.h"
#include "skinned_mesh.h"

#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"

#include <functional>
#include <map>

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

		loadNodes();
		loadAnimations();
		loadMaterials();
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

	struct Node {
		std::string name;
		std::vector<Node> children;
		aiNode* ai;
	};

	void DAELoader::loadNodes() {
		 std::function<void(aiNode*,Node&)> dfs = [&](aiNode* anode, Node& node) {
			 node.name = anode->mName.C_Str();
			 node.ai = anode;
			for (auto i = 0; i < anode->mNumChildren; ++i) {
				auto achild = anode->mChildren[i];
				Node child;
				dfs(achild, child);
				node.children.push_back(child);
			}
		};
		const auto aiRoot = scene->mRootNode;
		Node root;
		dfs(aiRoot, root);
	}

	void DAELoader::loadMaterials() {
		for (auto i = 0; i < scene->mNumMaterials; ++i) {
			const auto material = scene->mMaterials[i];
		}
	}
	
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
			
			// construct the geometry
			MeshGeometry* G = new MeshGeometry;
			if (mesh->HasPositions()) G->updateVertexData(vertices, 3 * mesh->mNumVertices);
			if (mesh->HasNormals()) G->updateNormalData(normals, 3 * mesh->mNumVertices);
			if (mesh->HasTextureCoords(0)) G->updateUVData(texcoords, 2 * mesh->mNumVertices);
			if (mesh->HasTangentsAndBitangents()) {
				G->updateTangentData(tangents, 3 * mesh->mNumVertices);
				G->updateBitangentData(bitangents, 3 * mesh->mNumVertices);
			}
			G->updateIndexData(&indices[0], indices.size());

			// TODO: use correct material
			ColorMaterial* M = new ColorMaterial;
			M->setAmbient(1.0f, 0.0f, 0.0f);

			// engine's mesh class
			SkinnedMesh* pathosMesh = new SkinnedMesh(G, M);
			std::vector<float> pos;
			pos.assign(vertices, vertices + mesh->mNumVertices * 3);
			pathosMesh->setInitialPositions(std::move(pos));

			// acquire transforms from the node hierarchy
			std::map<std::string, glm::mat4> nodeTransformMap;
			std::function<void(aiNode*, glm::mat4&)> calcTransform = [&](aiNode* node, glm::mat4& parent) {
				glm::mat4 T = parent * getGlmMat(node->mTransformation);
				nodeTransformMap[std::string(node->mName.C_Str())] = T;
				for (auto i = 0; i < node->mNumChildren; ++i) {
					calcTransform(node->mChildren[i], T);
				}
			};
			calcTransform(scene->mRootNode, glm::mat4(1.0f));

			// load bones
			if (mesh->HasBones()) {
				for (auto i = 0; i < mesh->mNumBones; ++i) {
					Bone bone;
					const auto aiBone = mesh->mBones[i];
					bone.name = aiBone->mName.C_Str();
					for (auto j = 0; j < aiBone->mNumWeights; ++j) {
						bone.vertexIDs.push_back(aiBone->mWeights[j].mVertexId);
						bone.weights.push_back(aiBone->mWeights[j].mWeight);
					}
					auto& nodeT = nodeTransformMap[bone.name];
					bone.offset = nodeT * getGlmMat(aiBone->mOffsetMatrix);
					pathosMesh->addBone(std::move(bone));
				}
			}
			// TODO: switch to hardware skinning
			pathosMesh->updateSoftwareSkinning();

			meshes.push_back(pathosMesh);
		}
	}

	void DAELoader::loadAnimations() {
		for (auto i = 0; i < scene->mNumAnimations; ++i) {
			const auto anim = scene->mAnimations[i];
			std::vector<aiNodeAnim*> xs;
			xs.assign(anim->mChannels, anim->mChannels + anim->mNumChannels);
		}
	}

}