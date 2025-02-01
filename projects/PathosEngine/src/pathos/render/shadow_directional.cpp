#include "shadow_directional.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/rhi/indirect_draw.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/mesh/mesh.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/util/log.h"
#include "pathos/console.h"

#include "badger/assertion/assertion.h"
#include "badger/types/matrix_types.h"
#include "badger/math/minmax.h"
#include "badger/math/vector_math.h"

// #todo-indirect-draw: Merge trivial drawcalls for CSM.
#define MERGE_TRIVIAL_DRAW_CALLS 1

namespace pathos {

	DirectionalShadowMap::~DirectionalShadowMap() {
		CHECKF(bDestroyed, "Resource leak");
	}

	void DirectionalShadowMap::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_CascadedShadowMap");

		uboPerFrame.init<UBO_PerFrame>("UBO_PerFrame_CSM");
		uboPerObject.init<Material::UBO_PerObject>("UBO_PerObject_CSM");
	}

	void DirectionalShadowMap::releaseResources(RenderCommandList& cmdList) {
		if (!bDestroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
			indirectDrawBuffer.reset();
			modelTransformBuffer.reset();
		}
		bDestroyed = true;
	}

	void DirectionalShadowMap::renderShadowMap(RenderCommandList& cmdList, SceneProxy* scene, const Camera* camera, Material* indirectDrawDummyMaterial, const UBO_PerFrame& cachedPerFrameUBOData) {
		SCOPED_DRAW_EVENT(CascadedShadowMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		static const GLfloat clear_depth_one[] = { 1.0f };

		sceneContext.reallocDirectionalShadowMaps(cmdList, scene->proxyList_directionalLight);

		uint32 numShadowCastingLights = 0;
		std::vector<size_t> lightIndices;
		lightIndices.reserve(scene->proxyList_directionalLight.size());
		for (size_t i = 0u; i < scene->proxyList_directionalLight.size(); ++i) {
			if (scene->proxyList_directionalLight[i]->bCastShadows) {
				numShadowCastingLights += 1;
				lightIndices.push_back(i);
			}
		}

		if (numShadowCastingLights == 0) {
			return; // Early exit if there is no shadow-casting directional lights.
		}

		cmdList.clipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.enable(GL_DEPTH_CLAMP); // Let vertices farther than zFar to be clamped to zFar.
		cmdList.depthFunc(GL_LESS);

		for (size_t lightIx : lightIndices) {
			SCOPED_DRAW_EVENT(CascadedShadowMapPerLight);

			DirectionalLightProxy* lightProxy = scene->proxyList_directionalLight[lightIx];

			if (lightProxy->bCastShadows == false) {
				continue;
			}

			const uint32 numCascades = lightProxy->shadowMapCascadeCount;

			cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
			cmdList.viewport(0, 0, lightProxy->shadowMapSize, lightProxy->shadowMapSize);

			for (uint32 cascadeIx = 0u; cascadeIx < numCascades; ++cascadeIx) {
				SCOPED_DRAW_EVENT(RenderCascade);

				cmdList.namedFramebufferTextureLayer(fbo, GL_DEPTH_ATTACHMENT, sceneContext.cascadedShadowMaps[lightIx], 0, cascadeIx);
				cmdList.clearBufferfv(GL_DEPTH, 0, clear_depth_one);
				pathos::checkFramebufferStatus(cmdList, fbo, "DirectionalShadowMap::renderShadowMap");

				// Hack uboPerFrame.
				{
					UBO_PerFrame uboData = cachedPerFrameUBOData;
					uboData.view = lightProxy->lightViewMatrices[cascadeIx];
					uboData.viewProj = lightProxy->lightViewProjMatrices[cascadeIx];
					uboData.temporalJitter = vector4(0.0f);

					uboPerFrame.update(cmdList, UBO_PerFrame::BINDING_POINT, &uboData);
				}

#if MERGE_TRIVIAL_DRAW_CALLS
				{
					const ShadowMeshProxyList& proxyList = scene->getTrivialShadowMeshes();
					const size_t numProxies = proxyList.size();
					const uint32 maxDrawcalls = (uint32)numProxies;
					
					const GLuint gPositionOnlyVAO = gRenderDevice->getPositionOnlyVAO();

					reallocateIndirectDrawBuffers(cmdList, maxDrawcalls);
					std::vector<DrawElementsIndirectCommand> drawCommands;
					std::vector<Material::UBO_PerObject> modelTransforms;
					drawCommands.reserve(maxDrawcalls);
					modelTransforms.reserve(maxDrawcalls);

					for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
						ShadowMeshProxy* proxy = proxyList[proxyIx];

						// #todo-frustum-culling: Frustum culling for CSM
						//if (bEnableFrustumCulling && !proxy->bInFrustum) {
						//	continue;
						//}

						DrawElementsIndirectCommand cmd{
							proxy->geometry->getIndexCount(),
							1, // instanceCount
							proxy->geometry->getFirstIndex(),
							(int32)proxy->geometry->getFirstVertex(), // baseVertex
							0, // baseInstance
						};
						drawCommands.emplace_back(cmd);

						Material::UBO_PerObject transforms;
						transforms.modelTransform = proxy->modelMatrix;
						transforms.prevModelTransform = proxy->modelMatrix; // Doesn't matter
						modelTransforms.emplace_back(transforms);
					}

					const uint32 mergedCalls = (uint32)drawCommands.size();
					if (mergedCalls > 0) {
						indirectDrawBuffer->writeToGPU_renderThread(cmdList, 0, sizeof(DrawElementsIndirectCommand) * mergedCalls, drawCommands.data());
						modelTransformBuffer->writeToGPU_renderThread(cmdList, 0, sizeof(Material::UBO_PerObject) * mergedCalls, modelTransforms.data());

						cmdList.useProgram(indirectDrawDummyMaterial->internal_getMaterialShader()->program->getGLName());

						cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, Material::UBO_PerObject::BINDING_POINT, modelTransformBuffer->internal_getGLName());
						cmdList.bindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectDrawBuffer->internal_getGLName());
						cmdList.bindVertexArray(gPositionOnlyVAO);

						cmdList.multiDrawElementsIndirect(
							GL_TRIANGLES,
							GL_UNSIGNED_INT, // 32-bit index
							0, // offset for indirect draw buffer
							mergedCalls,
							0 // stride
						);

						cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, Material::UBO_PerObject::BINDING_POINT, 0);
						cmdList.bindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
						cmdList.bindVertexArray(0);
					}
				}
#endif

				uint32 currentProgramHash = 0;
				uint32 currentMIID = 0xffffffff;

				for (ShadowMeshProxy* proxy : scene->getShadowMeshes()) {
					Material* material = proxy->material;
					MaterialShader* materialShader = material->internal_getMaterialShader();

#if MERGE_TRIVIAL_DRAW_CALLS
					// Drawcall for this proxy was auto-merged in the above block.
					if (proxy->bTrivialDepthOnly) {
						continue;
					}
#endif

					// #todo-frustum-culling: Frustum culling for CSM
					// ...

					bool bShouldBindProgram = (currentProgramHash != materialShader->programHash);
					bool bShouldUpdateMaterialParameters = (!materialShader->bTrivialDepthOnlyPass)
						&& (bShouldBindProgram || (currentMIID != material->internal_getMaterialInstanceID()));
					bool bUseWireframeMode = material->bWireframe;
					currentProgramHash = materialShader->programHash;
					currentMIID = material->internal_getMaterialInstanceID();

					if (bShouldBindProgram) {
						SCOPED_DRAW_EVENT(BindMaterialProgram);

						uint32 programName = materialShader->program->getGLName();
						CHECK(programName != 0 && programName != 0xffffffff);
						cmdList.useProgram(programName);
					}

					// Update UBO (per object)
					{
						Material::UBO_PerObject uboData;
						uboData.modelTransform = proxy->modelMatrix;
						uboData.prevModelTransform = proxy->modelMatrix; // Doesn't matter
						uboPerObject.update(cmdList, Material::UBO_PerObject::BINDING_POINT, &uboData);
					}

					// Update UBO (material)
					if (bShouldUpdateMaterialParameters && materialShader->uboTotalBytes > 0) {
						uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(materialShader->uboTotalBytes));
						material->internal_fillUniformBuffer(uboMemory);
						materialShader->uboMaterial.update(cmdList, materialShader->uboBindingPoint, uboMemory);
					}

					// #todo-material-assembler: How to detect if binding textures is mandatory?
					// Example cases:
					// - The vertex shader uses VTF(Vertex Texture Fetch)
					// - The pixel shader uses discard
					if (bShouldUpdateMaterialParameters) {
						for (const MaterialTextureParameter& mtp : material->internal_getTextureParameters()) {
							cmdList.bindTextureUnit(mtp.binding, mtp.texture->internal_getGLName());
						}
					}

					// #todo-renderer: Batching by same state
					if (proxy->doubleSided) cmdList.disable(GL_CULL_FACE);
					if (proxy->renderInternal) cmdList.frontFace(GL_CW);
					if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

					if (materialShader->bTrivialDepthOnlyPass) {
						proxy->geometry->bindPositionOnlyVAO(cmdList);
					} else {
						proxy->geometry->bindFullAttributesVAO(cmdList);
					}

					proxy->geometry->drawPrimitive(cmdList);

					// #todo-renderer: Batching by same state
					if (proxy->doubleSided || bUseWireframeMode) cmdList.enable(GL_CULL_FACE);
					if (proxy->renderInternal) cmdList.frontFace(GL_CCW);
					if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
			}
		}

		cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		cmdList.disable(GL_DEPTH_CLAMP);

		// #todo-renderer: Although it reverts the UBO properly, The debug name in RenderDoc appears as "UBO_PerFrame_CSM".
		// Revert uboPerFrame.
		uboPerFrame.update(cmdList, UBO_PerFrame::BINDING_POINT, (void*)&cachedPerFrameUBOData);
	}

	void DirectionalShadowMap::reallocateIndirectDrawBuffers(RenderCommandList& cmdList, uint32 maxDrawcalls) {
		auto getCount = [](Buffer* buffer, uint32 stride) -> uint32 {
			if (buffer == nullptr) return 0;
			return buffer->getCreateParams().bufferSize / stride;
		};

		if (maxDrawcalls > getCount(indirectDrawBuffer.get(), sizeof(DrawElementsIndirectCommand))) {
			BufferCreateParams createParams{
				EBufferUsage::CpuWrite,
				sizeof(DrawElementsIndirectCommand) * maxDrawcalls,
				nullptr, // initialData
				"Buffer_CSM_IndirectDrawArgs",
			};
			indirectDrawBuffer.reset();
			indirectDrawBuffer = makeUnique<Buffer>(createParams);
			indirectDrawBuffer->createGPUResource_renderThread(cmdList);
		}
		if (maxDrawcalls > getCount(modelTransformBuffer.get(), sizeof(Material::UBO_PerObject))) {
			BufferCreateParams createParams{
				EBufferUsage::CpuWrite,
				sizeof(Material::UBO_PerObject) * maxDrawcalls,
				nullptr, // initialData
				"Buffer_CSM_ModelTransforms",
			};
			modelTransformBuffer.reset();
			modelTransformBuffer = makeUnique<Buffer>(createParams);
			modelTransformBuffer->createGPUResource_renderThread(cmdList);
		}
	}

}
