#include "depth_prepass.h"
#include "scene_render_targets.h"
#include "scene_proxy.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/material/material_shader.h"
#include "pathos/material/material.h"
#include "pathos/mesh/geometry.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/camera.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

namespace pathos {

	void DepthPrepass::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_DepthPrepass");

		uboPerObject.init<Material::UBO_PerObject>("UBO_PerObject_DepthPrepass");
	}

	void DepthPrepass::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void DepthPrepass::renderPreDepth(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera)
	{
		SCOPED_DRAW_EVENT(DepthPrepass);
		
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

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

		static ConsoleVariableBase* cvarFrustum = ConsoleVariableManager::get().find("r.frustum_culling");
		CHECK(cvarFrustum != nullptr);
		const bool bEnableFrustumCulling = cvarFrustum->getInt() != 0;

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
					uboData.prevModelTransform = proxy->prevModelMatrix;
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
				if (proxy->doubleSided || bUseWireframeMode) cmdList.disable(GL_CULL_FACE);
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

}
