#include "daeloader.h"
#include "skinned_mesh.h"

#include "pathos/engine.h"
#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"
#include "pathos/loader/imageloader.h"
#include "pathos/util/resource_finder.h"

#include <functional>
#include <map>

namespace pathos {

	//////////////////////////////////////////////////////////////////////
	// public interface

	DAELoader::DAELoader() {}

	DAELoader::DAELoader(const char* filename, const char* material_dir, unsigned int flags, bool invertWinding) {
		materialDir = material_dir;
		if (materialDir[materialDir.size() - 1] != '/') materialDir += '/';
		load(filename, flags, invertWinding);
	}

	DAELoader::~DAELoader() {
		unload();
	}

	bool DAELoader::load(const char* filename, unsigned int flags, bool invertWinding) {
		if (scene) abort();

		std::string path = ResourceFinder::get().find(filename);
		scene = aiImportFile(path.c_str(), flags);
		if (!scene) return false;

		loadNodes();
		loadMaterials();
		loadMeshes(invertWinding);
		loadAnimations();

		// TODO: aiReleaseImport()

		return true;
	}

	bool DAELoader::unload() {
		if (!scene) return false;
		//aiReleaseImport(scene);
		scene = nullptr;
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// sub functions

	void DAELoader::loadNodes() {
		 std::function<void(aiNode*,Node*)> dfs = [&](aiNode* anode, Node* node) {
			 node->name = anode->mName.C_Str();
			 node->localTransform = getGlmMat(anode->mTransformation);
			for (auto i = 0u; i < anode->mNumChildren; ++i) {
				auto achild = anode->mChildren[i];
				Node* child = new Node;
				dfs(achild, child);
				node->children.push_back(child);
			}
		};
		const auto aiRoot = scene->mRootNode;
		root = new Node;
		dfs(aiRoot, root);
	}

	void DAELoader::loadMaterials() {
		auto getMaterialTextures = [&](const aiMaterial* M, aiTextureType texType) -> void {
			auto numTex = M->GetTextureCount(texType);
			for (auto i = 0u; i < numTex; ++i) {
				aiString texPath;
				M->GetTexture(texType, i, &texPath);
				if (textureMapping.find(texPath.C_Str()) != textureMapping.end()) {
					// this texture was already loaded
					continue;
				}
				std::string path = materialDir + texPath.C_Str();
				path = ResourceFinder::get().find(path);

				bool isSRGB = texType == aiTextureType_DIFFUSE;

				GLuint texture = pathos::createTextureFromBitmap(pathos::loadImage(path.c_str()), true, isSRGB);
				auto it = std::make_pair(texPath.C_Str(), texture);
				textureMapping.insert(it);
			}
		};
		for (auto i = 0u; i < scene->mNumMaterials; ++i) {
			const aiMaterial* M = scene->mMaterials[i];
			getMaterialTextures(M, aiTextureType_DIFFUSE);
			getMaterialTextures(M, aiTextureType_NORMALS);
		}
	}
	
	void DAELoader::loadMeshes(bool invertWinding) {
		SkinnedMesh* pathosMesh = new SkinnedMesh;

		for (auto i = 0u; i < scene->mNumMeshes; ++i) {
			const auto aiMeshIndex = i;
			const aiMesh* ai_mesh = scene->mMeshes[i];
			const auto N = ai_mesh->mNumVertices;

			// at least positions and indices are needed
			assert(ai_mesh->HasPositions() && ai_mesh->HasFaces());

			// TODO: support other primitives
			assert(ai_mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);

			std::vector<GLfloat> positions(N * 3);
			std::vector<GLfloat> normals(N * 3);
			std::vector<GLfloat> texcoords(N * 2);
			std::vector<GLfloat> tangents(N * 3);
			std::vector<GLfloat> bitangents(N * 3);

			// fill vertex buffer
			auto bufferSize = sizeof(GLfloat) * 3 * N;
			memcpy_s(positions.data(), bufferSize, reinterpret_cast<GLfloat*>(&ai_mesh->mVertices[0]), bufferSize);
			if (ai_mesh->HasNormals()) {
				memcpy_s(normals.data(), bufferSize, reinterpret_cast<GLfloat*>(&ai_mesh->mNormals[0]), bufferSize);
			}
			if (ai_mesh->HasTangentsAndBitangents()) {
				memcpy_s(tangents.data(), bufferSize, reinterpret_cast<GLfloat*>(&ai_mesh->mTangents[0]), bufferSize);
				memcpy_s(bitangents.data(), bufferSize, reinterpret_cast<GLfloat*>(&ai_mesh->mBitangents[0]), bufferSize);
			}
			if (ai_mesh->HasTextureCoords(0)) {
				for (auto i = 0u; i < N; ++i) {
					texcoords[i * 2 + 0] = ai_mesh->mTextureCoords[0][i].x;
					texcoords[i * 2 + 1] = ai_mesh->mTextureCoords[0][i].y;
				}
			}

			// fill index buffer
			std::vector<GLuint> indices;
			if (invertWinding) {
				for (auto i = 0u; i < ai_mesh->mNumFaces; ++i) {
					const aiFace& face = ai_mesh->mFaces[i];
					indices.push_back(face.mIndices[0]);
					indices.push_back(face.mIndices[2]);
					indices.push_back(face.mIndices[1]);
				}
			} else {
				for (auto i = 0u; i < ai_mesh->mNumFaces; ++i) {
					const aiFace& face = ai_mesh->mFaces[i];
					indices.push_back(face.mIndices[0]);
					indices.push_back(face.mIndices[1]);
					indices.push_back(face.mIndices[2]);
				}
			}
			
			// construct geometry
			MeshGeometry* G = new MeshGeometry;
			if (true || ai_mesh->HasPositions()) G->updatePositionData(positions.data(), 3 * N);
			if (true || ai_mesh->HasNormals()) G->updateNormalData(normals.data(), 3 * N);
			if (true || ai_mesh->HasTextureCoords(0)) G->updateUVData(texcoords.data(), 2 * N);
			if (true || ai_mesh->HasTangentsAndBitangents()) {
				G->updateTangentData(tangents.data(), 3 * N);
				G->updateBitangentData(bitangents.data(), 3 * N);
			}
			G->updateIndexData(&indices[0], (uint32_t)indices.size());

			// create material
			Material* M;
			const aiMaterial* ai_material = scene->mMaterials[ai_mesh->mMaterialIndex];
			bool hasDiffuseTexture = ai_material->GetTextureCount(aiTextureType_DIFFUSE) >= 1;
			bool hasNormalTexture = ai_material->GetTextureCount(aiTextureType_NORMALS) >= 1;
			if (hasDiffuseTexture && hasNormalTexture) {
				aiString diffusePath, normalPath;
				ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &diffusePath);
				ai_material->GetTexture(aiTextureType_NORMALS, 0, &normalPath);
				GLuint diffuseTex = textureMapping.find(diffusePath.C_Str())->second;
				GLuint normalTex = textureMapping.find(normalPath.C_Str())->second;
				M = pathos::createPBRMaterial(diffuseTex, normalTex);
			} else if (hasDiffuseTexture) {
				aiString diffusePath;
				ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &diffusePath);
				GLuint diffuseTex = textureMapping.find(diffusePath.C_Str())->second;
				M = pathos::createPBRMaterial(diffuseTex);
			} else {
				M = Material::createMaterialInstance("solid_color");
				M->setConstantParameter("albedo", vector3(0.9f, 0.0f, 0.0f));
				M->setConstantParameter("metallic", 0.0f);
				M->setConstantParameter("roughness", 0.8f);
				M->setConstantParameter("emissive", vector3(0.0f));
			}

			// add GM pair to the mesh
			pathosMesh->add(G, M);

			// set initial positions
			std::vector<float> pos;
			pos.assign(positions.data(), positions.data() + N * 3);
			pathosMesh->setInitialPositions(aiMeshIndex, std::move(pos));

			// load bones
			if (ai_mesh->HasBones()) {
				for (auto i = 0u; i < ai_mesh->mNumBones; ++i) {
					Bone bone;
					const auto aiBone = ai_mesh->mBones[i];
					bone.name = aiBone->mName.C_Str();
					for (auto j = 0u; j < aiBone->mNumWeights; ++j) {
						bone.vertexIDs.push_back(aiBone->mWeights[j].mVertexId);
						bone.weights.push_back(aiBone->mWeights[j].mWeight);
					}
					bone.offset = getGlmMat(aiBone->mOffsetMatrix);
					pathosMesh->addBone(aiMeshIndex, bone);
				}
			}
		}

		pathosMesh->setRoot(root);
		pathosMesh->updateSoftwareSkinning();

		mesh = pathosMesh;
	}

	void DAELoader::loadAnimations() {
		SkinnedMesh* skinnedMesh = dynamic_cast<SkinnedMesh*>(mesh);
		if (!skinnedMesh) {
			return;
		}
		for (auto i = 0u; i < scene->mNumAnimations; ++i) {
			const auto aiAnim = scene->mAnimations[i];
			SkeletalAnimation* anim = new SkeletalAnimation(aiAnim);
			skinnedMesh->addAnimation(anim);
		}
	}

}