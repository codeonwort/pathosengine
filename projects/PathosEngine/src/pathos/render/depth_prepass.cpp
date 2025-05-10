#include "depth_prepass.h"
#include "scene_render_targets.h"
#include "scene_proxy.h"
#include "landscape_rendering.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/rhi/indirect_draw.h"
#include "pathos/material/material.h"
#include "pathos/material/material_proxy.h"
#include "pathos/material/material_shader.h"
#include "pathos/mesh/geometry.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/camera.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

// #todo-indirect-draw: Merge trivial drawcalls to not stall render thread.
#define MERGE_TRIVIAL_DRAW_CALLS 1

namespace pathos {

	void DepthPrepass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_DepthPrepass");

		uboPerObject.init<MaterialProxy::UBO_PerObject>("UBO_PerObject_DepthPrepass");
	}

	void DepthPrepass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		indirectDrawBuffer.reset();
		modelTransformBuffer.reset();
	}

	void DepthPrepass::renderPreDepth(RenderCommandList& cmdList, SceneProxy* scene, Material* indirectDrawDummyMaterial, LandscapeRendering* landscapeRendering) {
		SCOPED_DRAW_EVENT(DepthPrepass);
		
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		static ConsoleVariableBase* cvarFrustum = ConsoleVariableManager::get().find("r.frustum_culling");
		CHECK(cvarFrustum != nullptr);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
			cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
			cmdList.depthFunc(GL_GREATER);
		} else {
			cmdList.depthFunc(GL_LESS);
		}
		cmdList.enable(GL_DEPTH_TEST);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		GLfloat* sceneDepthClearValue = (GLfloat*)cmdList.allocateSingleFrameMemory(sizeof(GLfloat));
		*sceneDepthClearValue = pathos::getDeviceFarDepth();
		cmdList.clearNamedFramebufferfv(fbo, GL_DEPTH, 0, sceneDepthClearValue);

		const bool bEnableFrustumCulling = cvarFrustum->getInt() != 0;

		landscapeRendering->renderLandscape(cmdList, scene, uboPerObject, true);

		// Draw every trivial opque static meshes at once.
#if MERGE_TRIVIAL_DRAW_CALLS
		{
			const std::vector<StaticMeshProxy*>& proxyList = scene->getTrivialDepthOnlyStaticMeshes();
			const size_t numProxies = proxyList.size();
			const uint32 maxDrawcalls = (uint32)numProxies;

			const GLuint gPositionOnlyVAO = gRenderDevice->getPositionOnlyVAO();

			reallocateIndirectDrawBuffers(cmdList, maxDrawcalls);
			std::vector<DrawElementsIndirectCommand> drawCommands;
			std::vector<MaterialProxy::UBO_PerObject> modelTransforms;
			drawCommands.reserve(maxDrawcalls);
			modelTransforms.reserve(maxDrawcalls);

			for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
				StaticMeshProxy* proxy = proxyList[proxyIx];
				MaterialProxy* material = proxy->material;
				MaterialShader* materialShader = material->materialShader;

				// Early out
				if (bEnableFrustumCulling && !proxy->bInFrustum) {
					continue;
				}

				DrawElementsIndirectCommand cmd{
					proxy->geometry->getIndexCount(),
					1, // instanceCount
					proxy->geometry->getFirstIndex(),
					(int32)proxy->geometry->getFirstVertex(), // baseVertex
					0, // baseInstance
				};
				drawCommands.emplace_back(cmd);

				MaterialProxy::UBO_PerObject transforms;
				transforms.modelTransform = proxy->modelMatrix;
				transforms.prevModelTransform = proxy->prevModelMatrix;
				modelTransforms.emplace_back(transforms);
			}

			const uint32 mergedCalls = (uint32)drawCommands.size();
			if (mergedCalls > 0) {
				indirectDrawBuffer->writeToGPU_renderThread(cmdList, 0, sizeof(DrawElementsIndirectCommand) * mergedCalls, drawCommands.data());
				modelTransformBuffer->writeToGPU_renderThread(cmdList, 0, sizeof(MaterialProxy::UBO_PerObject) * mergedCalls, modelTransforms.data());

				cmdList.useProgram(indirectDrawDummyMaterial->internal_getMaterialShader()->program->getGLName());
				
				cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, MaterialProxy::UBO_PerObject::BINDING_POINT, modelTransformBuffer->internal_getGLName());
				cmdList.bindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectDrawBuffer->internal_getGLName());
				cmdList.bindVertexArray(gPositionOnlyVAO);

				cmdList.multiDrawElementsIndirect(
					GL_TRIANGLES,
					GL_UNSIGNED_INT, // 32-bit index
					0, // offset for indirect draw buffer
					mergedCalls,
					0 // stride
				);

				cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, MaterialProxy::UBO_PerObject::BINDING_POINT, 0);
				cmdList.bindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
				cmdList.bindVertexArray(0);
			}
		}
#endif

		// Draw opaque static meshes.
		{
			const std::vector<StaticMeshProxy*>& proxyList = scene->getOpaqueStaticMeshes();
			const size_t numProxies = proxyList.size();
			uint32 currentProgramHash = 0;
			uint32 currentMIID = 0xffffffff;
			bool bCurrentWireframe = false;
			bool bCurrentReverseWinding = false;
			bool bCurrentDoubleSided = false;

			for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
				StaticMeshProxy* proxy = proxyList[proxyIx];
				MaterialProxy* material = proxy->material;
				MaterialShader* materialShader = material->materialShader;

#if MERGE_TRIVIAL_DRAW_CALLS
				// Drawcall for this proxy was auto-merged in the above block.
				if (proxy->bTrivialDepthOnly) {
					continue;
				}
#endif

				// Early out
				if (bEnableFrustumCulling && !proxy->bInFrustum) {
					continue;
				}

				bool bShouldBindProgram = (currentProgramHash != materialShader->programHash);
				bool bShouldUpdateMaterialParameters = (!materialShader->bTrivialDepthOnlyPass) && (bShouldBindProgram || (currentMIID != material->materialInstanceID));
				bool bShouldUpdateWireframe = material->bWireframe;
				bool bShouldUpdateWinding = bCurrentReverseWinding != proxy->renderInternal;
				bool bShouldUpdateDoubleSided = bCurrentDoubleSided != proxy->doubleSided;
				currentProgramHash = materialShader->programHash;
				currentMIID = material->materialInstanceID;
				bCurrentWireframe = material->bWireframe;
				bCurrentReverseWinding = proxy->renderInternal;
				bCurrentDoubleSided = proxy->doubleSided;

				if (bShouldBindProgram) {
					SCOPED_DRAW_EVENT(BindMaterialProgram);

					uint32 programName = materialShader->program->getGLName();
					CHECK(programName != 0 && programName != 0xffffffff);
					cmdList.useProgram(programName);
				}

				// Update UBO (per object)
				{
					MaterialProxy::UBO_PerObject uboData;
					uboData.modelTransform = proxy->modelMatrix;
					uboData.prevModelTransform = proxy->prevModelMatrix;
					uboPerObject.update(cmdList, MaterialProxy::UBO_PerObject::BINDING_POINT, &uboData);
				}

				// Update UBO (material)
				if (bShouldUpdateMaterialParameters && materialShader->uboTotalBytes > 0) {
					uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(materialShader->uboTotalBytes));
					material->fillUniformBuffer(uboMemory);
					materialShader->uboMaterial.update(cmdList, materialShader->uboBindingPoint, uboMemory);
				}

				// #todo-material-assembler: How to detect if binding textures is mandatory?
				// Example cases:
				// - The vertex shader uses VTF(Vertex Texture Fetch)
				// - The pixel shader uses discard
				if (bShouldUpdateMaterialParameters) {
					for (const MaterialTextureParameter& mtp : material->textureParameters) {
						cmdList.bindTextureUnit(mtp.binding, mtp.texture->internal_getGLName());
					}
				}

				if (bShouldUpdateWireframe) {
					cmdList.polygonMode(GL_FRONT_AND_BACK, bCurrentWireframe ? GL_LINE : GL_FILL);
					if (bCurrentWireframe) cmdList.disable(GL_CULL_FACE);
					else cmdList.enable(GL_CULL_FACE);
				}
				if (bShouldUpdateWinding) {
					cmdList.frontFace(bCurrentReverseWinding ? GL_CW : GL_CCW);
				}
				if (bShouldUpdateDoubleSided) {
					if (bCurrentDoubleSided) cmdList.disable(GL_CULL_FACE);
					else cmdList.enable(GL_CULL_FACE);
				}

				if (materialShader->bTrivialDepthOnlyPass) {
					proxy->geometry->bindPositionOnlyVAO(cmdList);
				} else {
					proxy->geometry->bindFullAttributesVAO(cmdList);
				}
				proxy->geometry->drawPrimitive(cmdList);
			}

			// Restore render state
			cmdList.enable(GL_CULL_FACE);
			cmdList.frontFace(GL_CCW);
			cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	void DepthPrepass::reallocateIndirectDrawBuffers(RenderCommandList& cmdList, uint32 maxDrawcalls) {
		auto getCount = [](Buffer* buffer, uint32 stride) -> uint32 {
			if (buffer == nullptr) return 0;
			return buffer->getCreateParams().bufferSize / stride;
		};

		if (maxDrawcalls > getCount(indirectDrawBuffer.get(), sizeof(DrawElementsIndirectCommand))) {
			BufferCreateParams createParams{
				EBufferUsage::CpuWrite,
				sizeof(DrawElementsIndirectCommand) * maxDrawcalls,
				nullptr, // initialData
				"Buffer_DepthPrepass_IndirectDrawArgs",
			};
			indirectDrawBuffer.reset();
			indirectDrawBuffer = makeUnique<Buffer>(createParams);
			indirectDrawBuffer->createGPUResource_renderThread(cmdList);
		}
		if (maxDrawcalls > getCount(modelTransformBuffer.get(), sizeof(MaterialProxy::UBO_PerObject))) {
			BufferCreateParams createParams{
				EBufferUsage::CpuWrite,
				sizeof(MaterialProxy::UBO_PerObject) * maxDrawcalls,
				nullptr, // initialData
				"Buffer_DepthPrepass_ModelTransforms",
			};
			modelTransformBuffer.reset();
			modelTransformBuffer = makeUnique<Buffer>(createParams);
			modelTransformBuffer->createGPUResource_renderThread(cmdList);
		}
	}

}
