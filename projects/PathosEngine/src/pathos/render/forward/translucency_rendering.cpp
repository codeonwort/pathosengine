#include "translucency_rendering.h"
#include "pathos/engine_policy.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/light/directional_light_component.h"
#include "pathos/light/point_light_component.h"
#include "pathos/console.h"

namespace pathos {

	static constexpr int32 MAX_DIRECTIONAL_LIGHTS = 2;
	static constexpr int32 MAX_POINT_LIGHTS = 8;

	struct UBO_LightInfo {
		static constexpr uint32 BINDING_POINT = 3;

		vector4i              numLightSources; // (directional, point, ?, ?)
		DirectionalLightProxy directionalLights[MAX_DIRECTIONAL_LIGHTS];
		PointLightProxy       pointLights[MAX_POINT_LIGHTS];
	};

}

namespace pathos {

	TranslucencyRendering::TranslucencyRendering()
	{
		uboPerObject.init<Material::UBO_PerObject>();
		uboLightInfo.init<UBO_LightInfo>();
	}

	void TranslucencyRendering::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
	}

	void TranslucencyRendering::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void TranslucencyRendering::renderTranslucency(
		RenderCommandList& cmdList,
		const SceneProxy* scene,
		const Camera* camera)
	{
		SCOPED_DRAW_EVENT(Translucency);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		bool bReverseZ = (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse);

		// renderstates for translucency
		{
			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

			cmdList.depthMask(GL_TRUE);
			cmdList.enable(GL_DEPTH_TEST);
			cmdList.depthFunc(bReverseZ ? GL_GREATER : GL_LESS);
		}

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		// #todo-translucency: Sort by z
		{
		}

		// #todo-translucency: temp light info forwarding
		{
			UBO_LightInfo uboData;
			uboData.numLightSources.x = std::min((int32)scene->proxyList_directionalLight.size(), MAX_DIRECTIONAL_LIGHTS);
			uboData.numLightSources.y = std::min((int32)scene->proxyList_pointLight.size(), MAX_POINT_LIGHTS);
			for (int32 i = 0; i < uboData.numLightSources.x; ++i) {
				uboData.directionalLights[i] = *(scene->proxyList_directionalLight[i]);
			}
			for (int32 i = 0; i < uboData.numLightSources.y; ++i) {
				uboData.pointLights[i] = *(scene->proxyList_pointLight[i]);
			}
			uboLightInfo.update(cmdList, UBO_LightInfo::BINDING_POINT, &uboData);
		}

		static ConsoleVariableBase* cvarFrustum = ConsoleVariableManager::get().find("r.frustum_culling");
		CHECK(cvarFrustum != nullptr);
		const bool bEnableFrustumCulling = cvarFrustum->getInt() != 0;

		uint32 currentProgramHash = 0;
		uint32 currentMIID = 0xffffffff;
		
		const auto& meshBatches = scene->getTranslucentStaticMeshes();
		for (const StaticMeshProxy* proxy : meshBatches) {
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
				uboPerObject.update(cmdList, Material::UBO_PerObject::BINDING_POINT, &uboData);
			}

			// Update UBO (material)
			if (bShouldUpdateMaterialParameters && materialShader->uboTotalBytes > 0) {
				uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(materialShader->uboTotalBytes));
				material->internal_fillUniformBuffer(uboMemory);
				materialShader->uboMaterial.update(cmdList, materialShader->uboBindingPoint, uboMemory);
			}

			// #todo-material-assembler: How to detect if binding textures is mandatory?
			// No translucent materials that use textures yet.
			//if (bShouldUpdateMaterialParameters) {
			//	for (const MaterialTextureParameter& mtp : material->internal_getTextureParameters()) {
			//		cmdList.bindTextureUnit(mtp.binding, mtp.glTexture);
			//	}
			//}

			proxy->geometry->activate_position_uv_normal_tangent_bitangent(cmdList);
			proxy->geometry->activateIndexBuffer(cmdList);
			proxy->geometry->drawPrimitive(cmdList);
		}

		// restore renderstates
		{
			cmdList.disable(GL_BLEND);
			cmdList.disable(GL_DEPTH_TEST);
		}
	}

}
