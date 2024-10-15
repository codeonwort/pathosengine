#include "gbuffer_pass.h"
#include "scene_render_targets.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/landscape_component.h"
#include "pathos/material/material.h"
#include "pathos/mesh/geometry.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

namespace pathos {
	
	struct UBO_LandscapeCulling {
		static constexpr uint32 BINDING_POINT = 1;

		vector4ui indexCountPerLOD;
		vector4ui firstIndexPerLOD;
		vector3   actorPosition;
		float     sectorSizeX;
		vector3   cameraPosition;
		float     sectorSizeY;
		uint32    sectorCountX;
		uint32    sectorCountY;
		float     _pad0;
		float     _pad1;
	};

	class LandscapeCullingCS : public ShaderStage {
	public:
		static const uint32 BUCKET_SIZE = 1024u;
		LandscapeCullingCS() : ShaderStage(GL_COMPUTE_SHADER, "LandscapeCullingCS") {
			addDefine("BUCKET_SIZE", BUCKET_SIZE);
			setFilepath("landscape_indirect_draw.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_LandscapeCulling, LandscapeCullingCS);

}

namespace pathos {

	GBufferPass::GBufferPass() {}
	GBufferPass::~GBufferPass() {}

	void GBufferPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_gbuffer");

		uboPerObject.init<Material::UBO_PerObject>("UBO_PerObject_GBufferPass");
		uboLandscapeCulling.init<UBO_LandscapeCulling>("UBO_LandscapeCulling");
	}

	void GBufferPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	// #note: Do this everytime as reallocSceneTextures() might recreate GL textures.
	void GBufferPass::updateFramebufferAttachments(RenderCommandList& cmdList, SceneRenderTargets* sceneRenderTargets) {
		GLenum gbuffer_draw_buffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
		};

		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneRenderTargets->gbufferA, 0);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT1, sceneRenderTargets->gbufferB, 0);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT2, sceneRenderTargets->gbufferC, 0);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT3, sceneRenderTargets->velocityMap, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneRenderTargets->sceneDepth, 0);
		cmdList.namedFramebufferDrawBuffers(fbo, _countof(gbuffer_draw_buffers), gbuffer_draw_buffers);

		pathos::checkFramebufferStatus(cmdList, fbo, "GBuffer setup is invalid");
	}

	void GBufferPass::renderGBuffers(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera, bool hasDepthPrepass) {
		SCOPED_DRAW_EVENT(GBufferPass);

		constexpr bool bReverseZ = pathos::getReverseZPolicy() == EReverseZPolicy::Reverse;

		SceneRenderTargets* sceneRenderTargets = cmdList.sceneRenderTargets;
		static ConsoleVariableBase* cvarFrustumCulling = ConsoleVariableManager::get().find("r.frustum_culling");
		CHECK(cvarFrustumCulling);

		updateFramebufferAttachments(cmdList, sceneRenderTargets);

		static const GLuint color_zero_ui[] = { 0, 0, 0, 0 };
		static const GLfloat color_zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat* sceneDepthClearValue = (GLfloat*)cmdList.allocateSingleFrameMemory(sizeof(GLfloat));
		*sceneDepthClearValue = pathos::getDeviceFarDepth();

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.clearNamedFramebufferuiv(fbo, GL_COLOR, 0, color_zero_ui);
		cmdList.clearNamedFramebufferfv(fbo, GL_COLOR, 1, color_zero);
		cmdList.clearNamedFramebufferfv(fbo, GL_COLOR, 2, color_zero);
		cmdList.clearNamedFramebufferfv(fbo, GL_COLOR, 3, color_zero);
		if (!hasDepthPrepass) {
			cmdList.clearNamedFramebufferfv(fbo, GL_DEPTH, 0, sceneDepthClearValue);
		}

		// Set render state
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.viewport(0, 0, sceneRenderTargets->sceneWidth, sceneRenderTargets->sceneHeight);

		// #todo-depth-prepass: Assume depth prepass renders every opaque objects.
		if (hasDepthPrepass) {
			cmdList.enable(GL_DEPTH_TEST);
			cmdList.depthFunc(GL_EQUAL);
			cmdList.depthMask(GL_FALSE);
		} else {
			cmdList.enable(GL_DEPTH_TEST);
			cmdList.depthFunc(bReverseZ ? GL_GEQUAL : GL_LEQUAL);
			cmdList.depthMask(GL_TRUE);
		}

		bool bEnableFrustumCulling = cvarFrustumCulling->getInt() != 0;

		// #wip-landscape: Very redundant with static mesh rendering.
		// Draw landscape
		{
			const std::vector<LandscapeProxy*>& proxyList = scene->getLandscapeMeshes();
			const size_t numProxies = proxyList.size();

			// #wip-indirect-draw: Run landscape culling shader
			bool bPrepareGpuDriven = false;
			for (size_t proxyIx = 0; proxyIx < numProxies && !bPrepareGpuDriven; ++proxyIx) {
				bPrepareGpuDriven = bPrepareGpuDriven || proxyList[proxyIx]->bGpuDriven;
			}
			if (bPrepareGpuDriven) {
				SCOPED_DRAW_EVENT(LandscapeGpuCulling);

				ShaderProgram& cullingProgram = FIND_SHADER_PROGRAM(Program_LandscapeCulling);
				cmdList.useProgram(cullingProgram.getGLName());

				for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
					LandscapeProxy* proxy = proxyList[proxyIx];
					if (proxy->bGpuDriven == false) continue;

					UBO_LandscapeCulling uboData;
					uboData.indexCountPerLOD = proxy->indexCountPerLOD;
					uboData.firstIndexPerLOD = proxy->firstIndexPerLOD;
					uboData.actorPosition    = proxy->actorPosition;
					uboData.sectorSizeX      = proxy->sectorSizeX;
					uboData.cameraPosition   = camera->getPosition();
					uboData.sectorSizeY      = proxy->sectorSizeY;
					uboData.sectorCountX     = proxy->sectorCountX;
					uboData.sectorCountY     = proxy->sectorCountY;
					uboData._pad0            = 0.0f;
					uboData._pad1            = 0.0f;

					uboLandscapeCulling.update(cmdList, UBO_LandscapeCulling::BINDING_POINT, &uboData);

					proxy->sectorParameterBuffer->bindAsSSBO(cmdList, 0);
					proxy->indirectDrawArgsBuffer->bindAsSSBO(cmdList, 1);

					uint32 groupSize = (proxy->indirectDrawCount + LandscapeCullingCS::BUCKET_SIZE - 1) / LandscapeCullingCS::BUCKET_SIZE;
					cmdList.dispatchCompute(groupSize, 1, 1);
				}
			}

			uint32 currentProgramHash = 0;
			uint32 currentMIID = 0xffffffff;

			for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
				LandscapeProxy* proxy = proxyList[proxyIx];
				Material* material = proxy->material;
				MaterialShader* materialShader = material->internal_getMaterialShader();

				// #wip-landscape: Frustum culling
				// ...

				bool bShouldBindProgram = (currentProgramHash != materialShader->programHash);
				bool bShouldUpdateMaterialParameters = bShouldBindProgram || (currentMIID != material->internal_getMaterialInstanceID());
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
					uboData.prevModelTransform = proxy->prevModelMatrix;
					uboPerObject.update(cmdList, Material::UBO_PerObject::BINDING_POINT, &uboData);
				}

				// Update UBO (material)
				if (bShouldUpdateMaterialParameters && materialShader->uboTotalBytes > 0) {
					uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(materialShader->uboTotalBytes));
					material->internal_fillUniformBuffer(uboMemory);
					materialShader->uboMaterial.update(cmdList, materialShader->uboBindingPoint, uboMemory);
				}

				// Bind texture units
				if (bShouldUpdateMaterialParameters) {
					for (const MaterialTextureParameter& mtp : material->internal_getTextureParameters()) {
						cmdList.bindTextureUnit(mtp.binding, mtp.texture->internal_getGLName());
					}
				}

				// #todo-renderer: Batching by same state
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

				proxy->geometry->bindFullAttributesVAO(cmdList);

				constexpr uint32 SSBO_BINDING_INDEX = 0;
				proxy->sectorParameterBuffer->bindAsSSBO(cmdList, SSBO_BINDING_INDEX);
				cmdList.bindBuffer(GL_DRAW_INDIRECT_BUFFER, proxy->indirectDrawArgsBuffer->internal_getGLName());

				cmdList.multiDrawElementsIndirect(
					GL_TRIANGLES,
					proxy->geometry->isIndex16Bit() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
					0, // offset for indirect draw buffer
					proxy->indirectDrawCount,
					0 // stride
				);

				// Unbind buffers
				cmdList.bindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
				cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_BINDING_INDEX, 0);

				// #todo-renderer: Batching by same state
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		// Draw opaque static meshes
		{
			const std::vector<StaticMeshProxy*>& proxyList = scene->getOpaqueStaticMeshes();
			const size_t numProxies = proxyList.size();
			uint32 currentProgramHash = 0;
			uint32 currentMIID = 0xffffffff;
			
			for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
				StaticMeshProxy* proxy = proxyList[proxyIx];
				Material* material = proxy->material;
				MaterialShader* materialShader = material->internal_getMaterialShader();

				// Early out
				if (bEnableFrustumCulling && !proxy->bInFrustum) {
					continue;
				}

				bool bShouldBindProgram = (currentProgramHash != materialShader->programHash);
				bool bShouldUpdateMaterialParameters = bShouldBindProgram || (currentMIID != material->internal_getMaterialInstanceID());
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
					uboData.prevModelTransform = proxy->prevModelMatrix;
					uboPerObject.update(cmdList, Material::UBO_PerObject::BINDING_POINT, &uboData);
				}

				// Update UBO (material)
				if (bShouldUpdateMaterialParameters && materialShader->uboTotalBytes > 0) {
					uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(materialShader->uboTotalBytes));
					material->internal_fillUniformBuffer(uboMemory);
					materialShader->uboMaterial.update(cmdList, materialShader->uboBindingPoint, uboMemory);
				}

				// Bind texture units
				if (bShouldUpdateMaterialParameters) {
					for (const MaterialTextureParameter& mtp : material->internal_getTextureParameters()) {
						cmdList.bindTextureUnit(mtp.binding, mtp.texture->internal_getGLName());
					}
				}

				// #todo-renderer: Batching by same state
				if (proxy->doubleSided || bUseWireframeMode) cmdList.disable(GL_CULL_FACE);
				if (proxy->renderInternal) cmdList.frontFace(GL_CW);
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);
				
				proxy->geometry->bindFullAttributesVAO(cmdList);
				proxy->geometry->drawPrimitive(cmdList);

				// #todo-renderer: Batching by same state
				if (proxy->doubleSided || bUseWireframeMode) cmdList.enable(GL_CULL_FACE);
				if (proxy->renderInternal) cmdList.frontFace(GL_CCW);
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		// Restore render state
		cmdList.depthMask(GL_TRUE);
	}

}
