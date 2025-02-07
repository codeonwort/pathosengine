#include "gltf_loader.h"
#include "image_loader.h"
#include "pathos/engine.h"
#include "pathos/rhi/texture.h"
#include "pathos/material/material.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/scene/directional_light_component.h"
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
		LOG(LogInfo, "[GLTF] Lights: %u", (uint32)tinyModel->lights.size());
		LOG(LogInfo, "[GLTF] Nodes: %u", (uint32)tinyModel->nodes.size());
		LOG(LogInfo, "[GLTF] Scenes: %u", (uint32)tinyModel->scenes.size());

		parseTextures(tinyModel.get());
		parseMaterials(tinyModel.get());
		parseMeshes(tinyModel.get());
		parseLights(tinyModel.get());

		const size_t totalNodes = tinyModel->nodes.size();
		transformParentIx.resize(totalNodes, -1);
		for (size_t nodeIx = 0; nodeIx < totalNodes; ++nodeIx) {
			const tinygltf::Node& tinyNode = tinyModel->nodes[nodeIx];

			for (int32 child : tinyNode.children) {
				transformParentIx[child] = (int32)nodeIx;
			}

			GLTFModelDesc desc{};
			desc.name = tinyNode.name;
			// Mesh
			desc.mesh = (tinyNode.mesh != -1) ? meshes[tinyNode.mesh] : nullptr;
			// Transform
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
				// GLTF rotation is (x, y, z, w) but glm::quat is (w, x, y, z)
				glm::quat q((float)tinyNode.rotation[3], (float)tinyNode.rotation[0], (float)tinyNode.rotation[1], (float)tinyNode.rotation[2]);
				vector3 rot = glm::eulerAngles(q); // Maps to ZYX rotation ( why not documented :/ )

				desc.rotation.pitch = glm::degrees(rot.x);
				desc.rotation.yaw = glm::degrees(rot.y);
				desc.rotation.roll = glm::degrees(rot.z);
				desc.rotation.convention = RotatorConvention::ZYX;
			}
			if (tinyNode.matrix.size() >= 16) {
				// #todo-gltf: Parse matrix
				LOG(LogWarning, "[GLTF] Should parse matrix");
			}
			// Extensions
			if (tinyNode.extensions.size() > 0) {
				auto lightIt = tinyNode.extensions.find("KHR_lights_punctual");
				if (lightIt != tinyNode.extensions.end()) {
					// Index in tinyModel->lights
					desc.lightIndex = lightIt->second.Get("light").GetNumberAsInt();
				}
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
			constexpr uint32 mipLevels = 0;
			constexpr bool autoDestroy = false;

			pending.glTexture = ImageUtils::createTexture2DFromImage(pending.blob, mipLevels, pending.sRGB, autoDestroy, pending.debugName.c_str());
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

			if (pending.tangentBlob != nullptr) {
				geometry->updateTangentData((float*)pending.tangentBlob, pending.tangentLength);
				if (pending.bShouldFreeTangent) {
					delete[] pending.tangentBlob;
				}
				geometry->calculateBitangentOnly();
			} else {
				geometry->calculateTangentBasis();
			}
		}
	}

	void GLTFLoader::attachToActor(Actor* targetActor, std::vector<SceneComponent*>* outComponents) {
		finalizeGPUUpload();

		uint32 numStaticMeshComponents = 0;
		uint32 numPointLightComponents = 0;
		uint32 numDirectionalLightComponents = 0;
		uint32 numSceneComponents = 0; // Placeholders not recognized as concrete components.
		uint32 numSkipped = 0; // tinyNodes not referenced by tinyScene.

		std::vector<SceneComponent*> comps(numModels(), nullptr);
		if (outComponents != nullptr) {
			outComponents->clear();
		}
		for (size_t i = 0; i < numModels(); ++i) {
			const GLTFModelDesc& desc = getModel(i);
			if (desc.bReferencedByScene == false) {
				++numSkipped;
				if (outComponents != nullptr) {
					outComponents->push_back(nullptr);
				}
				continue;
			}

			if (desc.mesh != nullptr) {
				comps[i] = new StaticMeshComponent;
				StaticMeshComponent* smc = static_cast<StaticMeshComponent*>(comps[i]);
				smc->setStaticMesh(desc.mesh);
				++numStaticMeshComponents;
			} else if (desc.lightIndex != -1) {
				const auto& lightDesc = pendingLights[desc.lightIndex];
				if (lightDesc.type == GLTFPendingLight::EType::Point) {
					comps[i] = new PointLightComponent;
					PointLightComponent* lightComp = static_cast<PointLightComponent*>(comps[i]);
					lightComp->color = lightDesc.point.color;
					lightComp->intensity = lightDesc.point.intensity / (3.14f * 4.0f);
					lightComp->attenuationRadius = lightDesc.point.attenuationRadius;
					++numPointLightComponents;
				} else if (lightDesc.type == GLTFPendingLight::EType::Directional) {
					comps[i] = new DirectionalLightComponent;
					DirectionalLightComponent* lightComp = static_cast<DirectionalLightComponent*>(comps[i]);
					lightComp->color = lightDesc.directional.color;
					lightComp->illuminance = lightDesc.directional.intensity;
					lightComp->direction = matrix3(desc.rotation.toMatrix()) * vector3(0, -1, 0);
					lightComp->direction.y *= -1;
					++numDirectionalLightComponents;
				} else {
					// #todo-gltf: Spot and area lights
					LOG(LogWarning, "[GLTF] Spot and area lights are not supported yet: %s", desc.name.c_str());
					comps[i] = new SceneComponent;
					++numSceneComponents;
				}
			} else {
				comps[i] = new SceneComponent;
				++numSceneComponents;
			}
			comps[i]->setLocation(desc.translation);
			comps[i]->setScale(desc.scale);
			comps[i]->setRotation(desc.rotation);

			targetActor->registerComponent(comps[i]);
			if (outComponents != nullptr) {
				outComponents->push_back(comps[i]);
			}
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

		LOG(LogInfo, "[GLTF] Try to create %u components:", (uint32)numModels());
		LOG(LogInfo, "[GLTF] - Static meshes      : %u", numStaticMeshComponents);
		LOG(LogInfo, "[GLTF] - Point lights       : %u", numPointLightComponents);
		LOG(LogInfo, "[GLTF] - Directional lights : %u", numDirectionalLightComponents);
		LOG(LogInfo, "[GLTF] - Placeholders       : %u", numSceneComponents);
		LOG(LogInfo, "[GLTF] - Skipped            : %u", numSkipped);
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

			ImageBlob* blob = new ImageBlob;
			blob->copyRawBytes(tinyImg.image.data(), tinyImg.width, tinyImg.height, bpp);
			if (bpp == 24) {
				blob->glStorageFormat = GL_RGB8;
				blob->glPixelFormat = GL_RGB;
				blob->glDataType = GL_UNSIGNED_BYTE;
			} else if (bpp == 32) {
				blob->glStorageFormat = GL_RGBA8;
				blob->glPixelFormat = GL_RGBA;
				blob->glDataType = GL_UNSIGNED_BYTE;
			}

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

				Texture* NORM = gEngine->getSystemTexture2DNormalmap();
				Texture* WHITE = gEngine->getSystemTexture2DWhite();

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
			StaticMesh* mesh = new StaticMesh;

			// For each mesh section
			for (size_t primIx = 0; primIx < tinyMesh.primitives.size(); ++primIx) {
				const tinygltf::Primitive& tinyPrim = tinyMesh.primitives[primIx];

				MeshGeometry* geometry = new MeshGeometry;
				geometry->initializeVertexLayout(MeshGeometry::EVertexAttributes::All, true);
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
					{
						auto it_uv0 = tinyPrim.attributes.find("TEXCOORD_0");
						uint8* uvBlob = nullptr;
						bool bUseTempUV = false;
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
								bUseTempUV = true;
								tempUV = new float[numPos * 2];
								for (uint32 i = 0; i < numPos; ++i) {
									float* curr = (float*)uvBlob;
									tempUV[i * 2 + 0] = curr[0];
									tempUV[i * 2 + 1] = 1.0f - curr[1];
									uvBlob += uvView.byteStride;
								}
							}
						} else {
							bUseTempUV = true;
							tempUV = new float[numPos * 2];
							for (uint32 i = 0; i < numPos; ++i) {
								tempUV[i * 2 + 0] = tempUV[i * 2 + 1] = 0.0f;
							}
						}
						pending.uvBlob = bUseTempUV ? (void*)tempUV : (void*)uvBlob;
						pending.uvLength = numPos * 2;
						pending.bShouldFreeUV = bUseTempUV;
						pending.bFlipTexcoordY = bFlipTexcoordY;
						// #todo-gltf: Other UV channels
						if (tinyPrim.attributes.find("TEXCOORD_1") != tinyPrim.attributes.end()) {
							//LOG(LogDebug, "[GLTF] Need to parse TEXCOORD_1");
						}
					}

					// Normal buffer
					{
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
					}

					// Tangent buffer
					// NOTE: Blender does not export tangents for non-triangulated meshes.
					{
						auto it_tan = tinyPrim.attributes.find("TANGENT");
						uint8* tanBlob = nullptr;
						uint32 tanLength = 0;
						bool bTempTangent = false;
						float* tempT = nullptr;
						if (it_tan != tinyPrim.attributes.end()) {
							const tinygltf::Accessor& tanDesc = tinyModel->accessors[it_tan->second];
							const tinygltf::BufferView& tanView = tinyModel->bufferViews[tanDesc.bufferView];
							CHECK(tanDesc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							CHECK(tanDesc.type == TINYGLTF_TYPE_VEC4);

							tanBlob = getBlobPtr(tanDesc);
							tanLength = (uint32)tanDesc.count * 4;
							if (tanView.byteStride != 0) {
								bTempTangent = true;
								tempT = new float[tanLength];
								for (uint32 i = 0; i < numPos; ++i) {
									float* curr = (float*)tanBlob;
									tempT[i * 3 + 0] = curr[0];
									tempT[i * 3 + 1] = curr[1];
									tempT[i * 3 + 2] = curr[2];
									tempT[i * 3 + 3] = 1.0f; // #todo-geometry: tangent w
									tanBlob += tanView.byteStride;
								}
							}
						}
						pending.tangentBlob = bTempTangent ? (void*)tempT : (void*)tanBlob;
						pending.tangentLength = tanLength;
						pending.bShouldFreeTangent = bTempTangent;
					}

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

	void GLTFLoader::parseLights(tinygltf::Model* tinyModel) {
		for (size_t lightIx = 0; lightIx < tinyModel->lights.size(); ++lightIx) {
			const tinygltf::Light& tinyLight = tinyModel->lights[lightIx];
			GLTFPendingLight light;

			if (tinyLight.type == "point") {
				light.type = GLTFPendingLight::EType::Point;
				light.point.color = vector3(tinyLight.color[0], tinyLight.color[1], tinyLight.color[2]);
				light.point.intensity = (float)tinyLight.intensity;
				light.point.attenuationRadius = (float)tinyLight.range;
			} else if (tinyLight.type == "directional") {
				light.type = GLTFPendingLight::EType::Directional;
				light.point.color = vector3(tinyLight.color[0], tinyLight.color[1], tinyLight.color[2]);
				light.point.intensity = (float)tinyLight.intensity;
			} else {
				// #todo-gltf: "spot" and "area"
				LOG(LogWarning, "[GLTF] Light type not supported yet: %s (%s)", tinyLight.type.data(), tinyLight.name.data());
				continue;
			}

			pendingLights.emplace_back(light);
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
