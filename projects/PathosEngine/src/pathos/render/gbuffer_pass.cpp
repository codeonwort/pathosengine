#include "gbuffer_pass.h"
#include "scene_render_targets.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/material/material.h"
#include "pathos/mesh/geometry.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

namespace pathos {

	GBufferPass::GBufferPass() {}
	GBufferPass::~GBufferPass() {}

	void GBufferPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_gbuffer");

		uboPerObject.init<Material::UBO_PerObject>("UBO_PerObject_GBufferPass");
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

		cmdList.depthMask(GL_TRUE);
	}

}
