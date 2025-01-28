#include "depth_prepass.h"
#include "scene_render_targets.h"
#include "scene_proxy.h"
#include "landscape_rendering.h"
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

// #todo-depth-prepass: Merge trivial drawcalls to not stall render thread. Same for shadowmap pass.

namespace pathos {

	void DepthPrepass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_DepthPrepass");

		uboPerObject.init<Material::UBO_PerObject>("UBO_PerObject_DepthPrepass");
	}

	void DepthPrepass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void DepthPrepass::renderPreDepth(RenderCommandList& cmdList, SceneProxy* scene, LandscapeRendering* landscapeRendering) {
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
				Material* material = proxy->material;
				MaterialShader* materialShader = material->internal_getMaterialShader();

				// Early out
				if (bEnableFrustumCulling && !proxy->bInFrustum) {
					continue;
				}

				bool bShouldBindProgram = (currentProgramHash != materialShader->programHash);
				bool bShouldUpdateMaterialParameters = (!materialShader->bTrivialDepthOnlyPass) && (bShouldBindProgram || (currentMIID != material->internal_getMaterialInstanceID()));
				bool bShouldUpdateWireframe = material->bWireframe;
				bool bShouldUpdateWinding = bCurrentReverseWinding != proxy->renderInternal;
				bool bShouldUpdateDoubleSided = bCurrentDoubleSided != proxy->doubleSided;
				currentProgramHash = materialShader->programHash;
				currentMIID = material->internal_getMaterialInstanceID();
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

}
