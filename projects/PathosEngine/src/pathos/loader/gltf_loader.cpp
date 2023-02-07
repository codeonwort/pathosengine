#include "gltf_loader.h"
#include "imageloader.h"
#include "pathos/engine.h"
#include "pathos/material/material.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include <tiny_gltf.h>
#include <glm/gtc/quaternion.hpp>

namespace pathos {

	GLTFLoader::GLTFLoader() {
		tinyLoader = makeUnique<tinygltf::TinyGLTF>();
		tinyModel = makeUnique<tinygltf::Model>();
	}

	GLTFLoader::~GLTFLoader() {
		LOG(LogDebug, "[GLTF] Destroy GLTFLoader");
	}

	bool GLTFLoader::loadASCII(const char* inFilename) {
		std::string filename = ResourceFinder::get().find(inFilename);

		if (filename.size() == 0) {
			LOG(LogError, "[GLTF] File not found: %s", inFilename);
			bIsValid = false;
			return false;
		}
		LOG(LogInfo, "[GLTF] Loading: %s", filename.c_str());

		std::string tinyErr, tinyWarn;

		bool ret = tinyLoader->LoadASCIIFromFile(tinyModel.get(), &tinyErr, &tinyWarn, filename);

		if (!tinyWarn.empty()) {
			LOG(LogWarning, tinyWarn.c_str());
		}
		if (!tinyErr.empty()) {
			LOG(LogError, tinyErr.c_str());
		}
		if (!ret) {
			LOG(LogError, "[GLTF] Failed to parse: %s", filename.c_str());
			bIsValid = false;
			return false;
		}

		fallbackMaterial = Material::createMaterialInstance("solid_color");
		fallbackMaterial->setConstantParameter("albedo", vector3(0.5f));
		fallbackMaterial->setConstantParameter("metallic", 0.0f);
		fallbackMaterial->setConstantParameter("roughness", 0.9f);
		fallbackMaterial->setConstantParameter("emissive", vector3(0.0f));

		LOG(LogInfo, "[GLTF] Textures: %u", (uint32)tinyModel->textures.size());
		LOG(LogInfo, "[GLTF] Materials: %u", (uint32)tinyModel->materials.size());
		LOG(LogInfo, "[GLTF] Meshes: %u", (uint32)tinyModel->meshes.size());
		LOG(LogInfo, "[GLTF] Nodes: %u", (uint32)tinyModel->nodes.size());
		LOG(LogInfo, "[GLTF] Scenes: %u", (uint32)tinyModel->scenes.size());

		parseTextures(tinyModel.get());
		parseMaterials(tinyModel.get());
		parseMeshes(tinyModel.get());

		const size_t totalNodes = tinyModel->nodes.size();
		transformParentIx.resize(totalNodes, -1);
		for (size_t nodeIx = 0; nodeIx < totalNodes; ++nodeIx) {
			const tinygltf::Node& tinyNode = tinyModel->nodes[nodeIx];

			for (int32 child : tinyNode.children) {
				transformParentIx[child] = (int32)nodeIx;
			}

			GLTFModelDesc desc{};
			desc.mesh = (tinyNode.mesh != -1) ? meshes[tinyNode.mesh] : nullptr;
			if (tinyNode.translation.size() >= 3) {
				desc.translation.x = (float)tinyNode.translation[0];
				desc.translation.y = (float)tinyNode.translation[1];
				desc.translation.z = (float)tinyNode.translation[2];
			}
			if (tinyNode.scale.size() >= 3) {
				desc.scale.x = (float)tinyNode.scale[0];
				desc.scale.y = (float)tinyNode.scale[1];
				desc.scale.z = (float)tinyNode.scale[2];
			}
			if (tinyNode.rotation.size() >= 4) {
				glm::quat q(
					(float)tinyNode.rotation[0], (float)tinyNode.rotation[1],
					(float)tinyNode.rotation[2], (float)tinyNode.rotation[3]);
				vector3 rot = glm::eulerAngles(q);
				// #todo-gltf: Rotation values mapping?
				desc.rotation.pitch = glm::degrees(rot.x) * 2.0f;
				desc.rotation.yaw = glm::degrees(rot.y) + 180.0f;
				desc.rotation.roll = glm::degrees(rot.z) * 2.0f;
				desc.rotation.clampValues();
				//LOG(LogDebug, "[GLTF] node=%u, pitch=%f yaw=%f roll=%f", nodeIx, desc.rotation.pitch, desc.rotation.yaw, desc.rotation.roll);
			}
			if (tinyNode.matrix.size() >= 16) {
				// #todo-gltf: Parse matrix
				LOG(LogWarning, "[GLTF] Should parse matrix");
			}

			finalModels.push_back(desc);
		}
		// #todo-gltf: Render only first scene.
		checkSceneReference(tinyModel.get(), 0, finalModels);

		LOG(LogInfo, "[GLTF] Done.");

		bIsValid = true;
		return true;
	}

	void GLTFLoader::finalizeGPUUpload() {
		for (GLTFPendingTexture& pending : pendingTextures) {
			constexpr bool genMips = true;
			constexpr bool autoDestroy = false;

			//LOG(LogDebug, "tex w=%u h=%u bpp=%u BGR=%d", pending.blob->width, pending.blob->height, pending.blob->bpp, pending.blob->bIsBGR);

			pending.glTexture = pathos::createTextureFromBitmap(
				pending.blob, genMips, pending.sRGB, pending.debugName.c_str(), autoDestroy);
		}
		for (GLTFPendingTextureParameter& param : pendingTextureParameters) {
			param.material->setTextureParameter(param.parameterName.c_str(),
				(param.index != -1) ? pendingTextures[param.index].glTexture : param.fallbackTexture);
		}
		for (GLTFPendingGeometry& pending : pendingGeometries) {
			MeshGeometry* geometry = pending.geometry;

			if (pending.bIndex16) {
				geometry->updateIndex16Data((uint16*)pending.indexBlob, pending.indexLength);
			} else {
				geometry->updateIndexData((uint32*)pending.indexBlob, pending.indexLength);
			}
			if (pending.bShouldFreeIndex) {
				delete[] pending.indexBlob;
			}

			//LOG(LogDebug, "[GLTF] Position temp=%d len=%u blob=%x", pending.bShouldFreePosition, pending.positionLength, pending.positionBlob);
			geometry->updatePositionData((float*)pending.positionBlob, pending.positionLength);
			if (pending.bShouldFreePosition) {
				delete[] pending.positionBlob;
			}

			geometry->updateUVData((float*)pending.uvBlob, pending.uvLength, pending.bFlipTexcoordY);
			if (pending.bShouldFreeUV) {
				delete[] pending.uvBlob;
			}

			if (pending.normalBlob != nullptr) {
				geometry->updateNormalData((float*)pending.normalBlob, pending.normalLength);
				if (pending.bShouldFreeNormal) {
					delete[] pending.normalBlob;
				}
			} else {
				geometry->calculateNormals();
			}

			// #todo-gltf: Parse tangent
			geometry->calculateTangentBasis();
		}
	}

	void GLTFLoader::attachToActor(Actor* targetActor) {
		finalizeGPUUpload();

		uint32 numStaticMeshComponents = 0;
		uint32 numSceneComponents = 0;
		uint32 numSkipped = 0; // tinyNodes not referenced by tinyScene.

		std::vector<SceneComponent*> comps(numModels(), nullptr);
		for (size_t i = 0; i < numModels(); ++i) {
			const GLTFModelDesc& desc = getModel(i);
			if (desc.bReferencedByScene == false) {
				++numSkipped;
				continue;
			}

			if (desc.mesh != nullptr) {
				comps[i] = new StaticMeshComponent;
				StaticMeshComponent* smc = static_cast<StaticMeshComponent*>(comps[i]);
				smc->setStaticMesh(desc.mesh);
				++numStaticMeshComponents;
			} else {
				comps[i] = new SceneComponent;
				++numSceneComponents;
			}
			comps[i]->setLocation(desc.translation);
			comps[i]->setScale(desc.scale);
			comps[i]->setRotation(desc.rotation);

			targetActor->registerComponent(comps[i]);
		}
		for (size_t i = 0; i < numModels(); ++i) {
			if (getModel(i).bReferencedByScene == false) {
				continue;
			}
			if (transformParentIx[i] == -1) {
				comps[i]->setTransformParent(targetActor->getRootComponent());
			} else {
				comps[i]->setTransformParent(comps[transformParentIx[i]]);
			}
		}

		LOG(LogInfo, "[GLTF] Num static meshes = %u, Num placeholders = %u, Num skipped = %u",
			numStaticMeshComponents, numSceneComponents, numSkipped);
	}

	void GLTFLoader::parseTextures(tinygltf::Model* tinyModel) {
		std::vector<bool> isSRGB(tinyModel->textures.size(), false);
		for (const tinygltf::Material& tinyMat : tinyModel->materials) {
			int32 texIx = tinyMat.pbrMetallicRoughness.baseColorTexture.index;
			if (texIx != -1) {
				isSRGB[texIx] = true;
			}
		}

		for (size_t texIx = 0; texIx < tinyModel->textures.size(); ++texIx) {
			const tinygltf::Texture& tinyTex = tinyModel->textures[texIx];
			const tinygltf::Image& tinyImg = tinyModel->images[tinyTex.source];
			CHECK(tinyImg.bits == 8);

			uint32 bpp = tinyImg.bits * tinyImg.component;
			CHECK(bpp == 24 || bpp == 32);
			CHECKF(tinyImg.image.size() == (tinyImg.width * tinyImg.height * tinyImg.component), "Image blob size is invalid");

			constexpr bool hasOpacity = false;
			BitmapBlob* blob = new BitmapBlob(
				(uint8*)(tinyImg.image.data()), tinyImg.width, tinyImg.height, bpp, hasOpacity);

			GLTFPendingTexture pending;
			pending.blob = blob;
			pending.sRGB = isSRGB[texIx];
			pending.debugName = tinyImg.uri;

			pendingTextures.emplace_back(pending);
		}
	}

	void GLTFLoader::parseMaterials(tinygltf::Model* tinyModel) {
		auto toVector3 = [](const std::vector<double>& tinyVec, const vector3& fallback) -> vector3 {
			if (tinyVec.size() >= 3) {
				return vector3(float(tinyVec[0]), float(tinyVec[1]), float(tinyVec[2]));
			}
			return fallback;
		};

		// #todo-gltf: MASK and BLEND materials
		uint32 numMasks = 0, numBlends = 0;

		for (size_t materialIx = 0; materialIx < tinyModel->materials.size(); ++materialIx) {
			Material* material = fallbackMaterial;

			const tinygltf::Material& tinyMat = tinyModel->materials[materialIx];

			if (tinyMat.alphaMode == "OPAQUE") {
				int32 baseColorId = tinyMat.pbrMetallicRoughness.baseColorTexture.index;
				int32 normalId = tinyMat.normalTexture.index;
				int32 metalRoughId = tinyMat.pbrMetallicRoughness.metallicRoughnessTexture.index;
				int32 localAOId = tinyMat.occlusionTexture.index;
				int32 emissiveId = tinyMat.emissiveTexture.index;

				// TEXCOORD_0
				CHECK(tinyMat.pbrMetallicRoughness.baseColorTexture.texCoord == 0);
				CHECK(tinyMat.normalTexture.texCoord == 0);
				CHECK(tinyMat.pbrMetallicRoughness.metallicRoughnessTexture.texCoord == 0);
				CHECK(tinyMat.occlusionTexture.texCoord == 0);
				CHECK(tinyMat.emissiveTexture.texCoord == 0);

				vector3 baseColorFactor = toVector3(tinyMat.pbrMetallicRoughness.baseColorFactor, vector3(1.0f));
				vector3 emissiveFactor = toVector3(tinyMat.emissiveFactor, vector3(0.0f));
				float metallicFactor = (float)tinyMat.pbrMetallicRoughness.metallicFactor;
				float roughnessFactor = (float)tinyMat.pbrMetallicRoughness.roughnessFactor;

				const GLuint NORM = gEngine->getSystemTexture2DNormalmap();
				const GLuint WHITE = gEngine->getSystemTexture2DWhite();

				material = Material::createMaterialInstance("gltf_opaque");
				material->setConstantParameter("baseColorFactor", baseColorFactor);
				material->setConstantParameter("metallicFactor", metallicFactor);
				material->setConstantParameter("roughnessFactor", roughnessFactor);
				material->setConstantParameter("emissiveFactor", emissiveFactor);

				pendingTextureParameters.emplace_back(
					GLTFPendingTextureParameter(material, "baseColorTexture", baseColorId, WHITE));
				pendingTextureParameters.emplace_back(
					GLTFPendingTextureParameter(material, "normalTexture", normalId, NORM));
				pendingTextureParameters.emplace_back(
					GLTFPendingTextureParameter(material, "metallicRoughnessTexture", metalRoughId, WHITE));
				pendingTextureParameters.emplace_back(
					GLTFPendingTextureParameter(material, "occlusionTexture", localAOId, WHITE));
				pendingTextureParameters.emplace_back(
					GLTFPendingTextureParameter(material, "emissiveTexture", emissiveId, WHITE));

			} else if (tinyMat.alphaMode == "MASK") {
				float alphaCutoff = (float)tinyMat.alphaCutoff;
				++numMasks;
			} else if (tinyMat.alphaMode == "BLEND") {
				++numBlends;
			}

			materials.push_back(material);
		}

		LOG(LogDebug, "[GLTF] Matrials not parsed: MASK=%u BLEND=%u", numMasks, numBlends);
	}

	void GLTFLoader::parseMeshes(tinygltf::Model* tinyModel) {
		// Primitive mode
		// 0 POINTS
		// 1 LINES
		// 2 LINE_LOOP
		// 3 LINE_STRIP
		// 4 TRIANGLES
		// 5 TRIANGLE_STRIP
		// 6 TRIANGLE_FAN

		auto getBlobPtr = [tinyModel](const tinygltf::Accessor& accessor) -> uint8* {
			const tinygltf::BufferView& view = tinyModel->bufferViews[accessor.bufferView];
			uint8* blob = tinyModel->buffers[view.buffer].data.data();
			blob += view.byteOffset + accessor.byteOffset;
			return blob;
		};

		// For each mesh
		for (size_t meshIx = 0; meshIx < tinyModel->meshes.size(); ++meshIx) {
			const tinygltf::Mesh& tinyMesh = tinyModel->meshes[meshIx];
			Mesh* mesh = new Mesh;

			// For each mesh section
			for (size_t primIx = 0; primIx < tinyMesh.primitives.size(); ++primIx) {
				const tinygltf::Primitive& tinyPrim = tinyMesh.primitives[primIx];
				MeshGeometry* geometry = new MeshGeometry;
				Material* material = fallbackMaterial;

				// #todo-gltf: Temp ignore mesh sections with unsupported materials.
				if (tinyPrim.material == -1 || materials[tinyPrim.material] == fallbackMaterial) {
					continue;
				}

				if (tinyPrim.mode == 4 && tinyPrim.indices != -1) {
					GLTFPendingGeometry pending;
					pending.geometry = geometry;

					// Index buffer
					{
						const tinygltf::Accessor& indicesDesc = tinyModel->accessors[tinyPrim.indices];
						CHECK(indicesDesc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
							|| indicesDesc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);

						const tinygltf::BufferView& indicesView = tinyModel->bufferViews[indicesDesc.bufferView];
						CHECK(indicesView.byteStride == 0); // Indices are consecutive, right?

						pending.indexBlob = getBlobPtr(indicesDesc);
						pending.indexLength = (uint32)indicesDesc.count;
						pending.bShouldFreeIndex = false;
						pending.bIndex16 = (indicesDesc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
					}

					// Position buffer
					uint32 numPos = 0;
					{
						auto it = tinyPrim.attributes.find("POSITION");
						const tinygltf::Accessor& posDesc = tinyModel->accessors[it->second];
						CHECK(posDesc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
						CHECK(posDesc.type == TINYGLTF_TYPE_VEC3);

						const tinygltf::BufferView& posView = tinyModel->bufferViews[posDesc.bufferView];

						uint8* posBlob = getBlobPtr(posDesc);
						numPos = (uint32)posDesc.count;
						bool bTempBlob = false;
						float* tempPos = nullptr;

						if (posView.byteStride != 0) {
							bTempBlob = true;
							tempPos = new float[numPos * 3];
							for (uint32 i = 0; i < numPos; ++i) {
								float* curr = (float*)posBlob;
								tempPos[i * 3 + 0] = curr[0];
								tempPos[i * 3 + 1] = curr[1];
								tempPos[i * 3 + 2] = curr[2];
								posBlob += posView.byteStride;
							}
						}
						
						pending.positionBlob = bTempBlob ? (void*)tempPos : (void*)posBlob;
						pending.positionLength = numPos * 3;
						pending.bShouldFreePosition = bTempBlob;
					}
					CHECK(numPos != 0);

					// Texcoord buffer
					auto it_uv0 = tinyPrim.attributes.find("TEXCOORD_0");
					uint8* uvBlob = nullptr;
					bool bTempUV = false;
					bool bFlipTexcoordY = false;
					float* tempUV = nullptr;
					if (it_uv0 != tinyPrim.attributes.end()) {
						const tinygltf::Accessor& uvDesc = tinyModel->accessors[it_uv0->second];
						const tinygltf::BufferView& uvView = tinyModel->bufferViews[uvDesc.bufferView];
						CHECK(uvDesc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
						CHECK(uvDesc.type == TINYGLTF_TYPE_VEC2);

						uvBlob = getBlobPtr(uvDesc);
						if (uvView.byteStride == 0) {
							bFlipTexcoordY = true;
						} else {
							tempUV = new float[numPos * 2];
							for (uint32 i = 0; i < numPos; ++i) {
								float* curr = (float*)uvBlob;
								tempUV[i * 2 + 0] = curr[0];
								tempUV[i * 2 + 1] = 1.0f - curr[1];
								uvBlob += uvView.byteStride;
							}
						}
					} else {
						tempUV = new float[numPos * 2];
						for (uint32 i = 0; i < numPos; ++i) {
							tempUV[i * 2 + 0] = tempUV[i * 2 + 1] = 0.0f;
						}
					}
					pending.uvBlob = bTempUV ? (void*)tempUV : (void*)uvBlob;
					pending.uvLength = numPos * 2;
					pending.bShouldFreeUV = bTempUV;
					pending.bFlipTexcoordY = bFlipTexcoordY;
					// #todo-gltf: Other UV channels
					if (tinyPrim.attributes.find("TEXCOORD_1") != tinyPrim.attributes.end()) {
						//LOG(LogDebug, "[GLTF] Need to parse TEXCOORD_1");
					}

					// Normal buffer
					auto it_n = tinyPrim.attributes.find("NORMAL");
					uint8* normBlob = nullptr;
					bool bTempNormal = false;
					float* tempN = nullptr;
					if (it_n != tinyPrim.attributes.end()) {
						const tinygltf::Accessor& normDesc = tinyModel->accessors[it_n->second];
						const tinygltf::BufferView& normView = tinyModel->bufferViews[normDesc.bufferView];
						CHECK(normDesc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
						CHECK(normDesc.type == TINYGLTF_TYPE_VEC3);

						normBlob = getBlobPtr(normDesc);
						if (normView.byteStride != 0) {
							bTempNormal = true;
							tempN = new float[numPos * 3];
							for (uint32 i = 0; i < numPos; ++i) {
								float* curr = (float*)normBlob;
								tempN[i * 3 + 0] = curr[0];
								tempN[i * 3 + 1] = curr[1];
								tempN[i * 3 + 2] = curr[2];
								normBlob += normView.byteStride;
							}
						}
					}
					pending.normalBlob = bTempNormal ? (void*)tempN : (void*)normBlob;
					pending.normalLength = numPos * 3;
					pending.bShouldFreeNormal = bTempNormal;

					pendingGeometries.push_back(pending);

					if (tinyPrim.material != -1) {
						material = materials[tinyPrim.material];
					}
					if (material == fallbackMaterial) {
						LOG(LogDebug, "[GLTF] Fallback material ref: mesh=%u prim=%u", meshIx, primIx);
					}

					mesh->add(geometry, material);
				} else {
					LOG(LogError, "[GLTF] Primitive type is not supported: mode=%d, indices=%d", tinyPrim.mode, tinyPrim.indices);
				}
			}

			meshes.push_back(mesh);
		}
	}

	void checkSceneReferencesSub(tinygltf::Model* tinyModel, std::vector<int>& nodes, std::vector<GLTFModelDesc>& finalModels) {
		for (int nodeIx : nodes) {
			finalModels[nodeIx].bReferencedByScene = true;
			if (tinyModel->nodes[nodeIx].children.size() > 0) {
				checkSceneReferencesSub(tinyModel, tinyModel->nodes[nodeIx].children, finalModels);
			}
		}
	}
	void GLTFLoader::checkSceneReference(tinygltf::Model* tinyModel, int32 sceneIndex, std::vector<GLTFModelDesc>& finalModels) {
		checkSceneReferencesSub(tinyModel, tinyModel->scenes[sceneIndex].nodes, finalModels);
	}

}
