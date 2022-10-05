#include "gltf_loader.h"
#include "imageloader.h"
#include "pathos/material/material.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include "tiny_gltf.h"


namespace pathos {

	bool GLTFLoader::loadASCII(const char* inFilename) {
		std::string filename = ResourceFinder::get().find(inFilename);

		tinygltf::TinyGLTF tinyLoader;
		tinygltf::Model tinyModel;
		std::string tinyErr, tinyWarn;

		bool ret = tinyLoader.LoadASCIIFromFile(&tinyModel, &tinyErr, &tinyWarn, filename);

		if (!tinyWarn.empty()) {
			LOG(LogWarning, tinyWarn.c_str());
		}
		if (!tinyErr.empty()) {
			LOG(LogError, tinyErr.c_str());
		}
		if (!ret) {
			LOG(LogError, "[GLTF] Failed to parse: %s", filename.c_str());
			return false;
		}

		fallbackMaterial = Material::createMaterialInstance("solid_color");
		fallbackMaterial->setConstantParameter("albedo", vector3(0.5f));
		fallbackMaterial->setConstantParameter("metallic", 0.0f);
		fallbackMaterial->setConstantParameter("roughness", 0.9f);
		fallbackMaterial->setConstantParameter("emissive", vector3(0.0f));

		parseTextures(&tinyModel);
		parseMaterials(&tinyModel);
		parseMeshes(&tinyModel);

		for (size_t nodeIx = 0; nodeIx < tinyModel.scenes[0].nodes.size(); ++nodeIx) {
			const tinygltf::Node& tinyNode = tinyModel.nodes[nodeIx];
			if (tinyNode.mesh == -1) {
				continue;
			}

			GLTFModelDesc desc{};
			desc.mesh = meshes[tinyNode.mesh];
			if (tinyNode.translation.size() > 0) {
				desc.translation.x = (float)tinyNode.translation[0];
				desc.translation.y = (float)tinyNode.translation[1];
				desc.translation.z = (float)tinyNode.translation[2];
			}
			if (tinyNode.scale.size() > 0) {
				desc.scale.x = (float)tinyNode.scale[0];
				desc.scale.y = (float)tinyNode.scale[1];
				desc.scale.z = (float)tinyNode.scale[2];
			}

			finalModels.push_back(desc);
		}

		return true;
	}

	void GLTFLoader::parseTextures(tinygltf::Model* tinyModel) {
		//for (size_t imageIx = 0; imageIx < tinyModel->images.size(); ++imageIx) {
		//	const tinygltf::Image& tinyImage = tinyModel->images[imageIx];
		//}
		for (size_t texIx = 0; texIx < tinyModel->textures.size(); ++texIx) {
			const tinygltf::Texture& tinyTex = tinyModel->textures[texIx];
			const tinygltf::Image& tinyImg = tinyModel->images[tinyTex.source];
			CHECK(tinyImg.bits == 8);

			uint32 bpp = tinyImg.bits * tinyImg.component;
			CHECK(bpp == 24 || bpp == 32);

			// #todo-gltf: Can't know sRGB here
			bool sRGB = false;
			constexpr bool hasOpacity = false;
			BitmapBlob blob((uint8*)(tinyImg.image.data()), tinyImg.width, tinyImg.height, bpp, hasOpacity);
			GLuint glTex = pathos::createTextureFromBitmap(&blob, true, sRGB, tinyImg.uri.c_str(), false);
			
			glTextures.push_back(glTex);
		}
	}

	void GLTFLoader::parseMaterials(tinygltf::Model* tinyModel) {
		for (size_t materialIx = 0; materialIx < tinyModel->materials.size(); ++materialIx) {
			Material* material = fallbackMaterial;

			// #todo-gltf: Parse materials
			const tinygltf::Material& tinyMat = tinyModel->materials[materialIx];
			if (tinyMat.alphaMode == "OPAQUE") {
				int32 albedoId = tinyMat.pbrMetallicRoughness.baseColorTexture.index;
				int32 metalRoughId = tinyMat.pbrMetallicRoughness.metallicRoughnessTexture.index;

				// TEXCOORD_0
				CHECK(tinyMat.pbrMetallicRoughness.baseColorTexture.texCoord == 0);
				CHECK(tinyMat.pbrMetallicRoughness.metallicRoughnessTexture.texCoord == 0);

				material = pathos::createPBRMaterial(glTextures[albedoId]);
			} else if (tinyMat.alphaMode == "MASK") {
				float alphaCutoff = (float)tinyMat.alphaCutoff;
				//
			} else if (tinyMat.alphaMode == "BLEND") {
				//
			}

			materials.push_back(material);
		}
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
			MeshGeometry* geometry = new MeshGeometry;
			Material* material = fallbackMaterial;

			// For each mesh section
			for (size_t primIx = 0; primIx < tinyMesh.primitives.size(); ++primIx) {
				const tinygltf::Primitive& tinyPrim = tinyMesh.primitives[primIx];
				if (tinyPrim.mode == 4 && tinyPrim.indices != -1) {

					// Index buffer
					{
						const tinygltf::Accessor& indicesDesc = tinyModel->accessors[tinyPrim.indices];
						CHECK(indicesDesc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
							|| indicesDesc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);

						const tinygltf::BufferView& indicesView = tinyModel->bufferViews[indicesDesc.bufferView];
						CHECK(indicesView.byteStride == 0); // Indices are consecutive, right?

						uint8* indicesBlob = getBlobPtr(indicesDesc);
						const uint32 numIndices = (uint32)indicesDesc.count;
						if (indicesDesc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
							geometry->updateIndex16Data((uint16*)indicesBlob, numIndices);
						} else {
							geometry->updateIndexData((uint32*)indicesBlob, numIndices);
						}
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
						if (posView.byteStride == 0) {
							geometry->updatePositionData((float*)posBlob, numPos * 3);
						} else {
							std::vector<float> tempPos;
							tempPos.reserve(numPos * 3);
							for (uint32 i = 0; i < numPos; ++i) {
								float* curr = (float*)posBlob;
								tempPos.push_back(curr[0]);
								tempPos.push_back(curr[1]);
								tempPos.push_back(curr[2]);
								posBlob += posView.byteStride;
							}
							geometry->updatePositionData(tempPos.data(), (uint32)tempPos.size());
						}
					}
					CHECK(numPos != 0);

					// Texcoord buffer
					auto it_uv0 = tinyPrim.attributes.find("TEXCOORD_0");
					if (it_uv0 != tinyPrim.attributes.end()) {
						const tinygltf::Accessor& uvDesc = tinyModel->accessors[it_uv0->second];
						const tinygltf::BufferView& uvView = tinyModel->bufferViews[uvDesc.bufferView];
						CHECK(uvDesc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
						CHECK(uvDesc.type == TINYGLTF_TYPE_VEC2);

						uint8* uvBlob = getBlobPtr(uvDesc);
						if (uvView.byteStride == 0) {
							constexpr bool flipY = true;
							geometry->updateUVData((float*)uvBlob, numPos * 2, flipY);
						} else {
							std::vector<float> tempUV;
							tempUV.reserve(numPos * 2);
							for (uint32 i = 0; i < numPos; ++i) {
								float* curr = (float*)uvBlob;
								tempUV.push_back(curr[0]);
								tempUV.push_back(1.0f - curr[1]);
								uvBlob += uvView.byteStride;
							}
							geometry->updateUVData(tempUV.data(), (uint32)tempUV.size());
						}
					} else {
						std::vector<vector2> texcoords(numPos, vector2(0.0f));
						geometry->updateUVData((float*)texcoords.data(), numPos * 2);
					}

					// #todo-gltf: Parse normal, tangent
					geometry->calculateNormals();
					geometry->calculateTangentBasis();

					if (tinyPrim.material != -1) {
						material = materials[tinyPrim.material];
					}

					mesh->add(geometry, material);
				}
			}

			meshes.push_back(mesh);
		}
	}

}
