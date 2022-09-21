#include "translucency_rendering.h"
#include "pathos/engine_policy.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/light/directional_light_component.h"
#include "pathos/light/point_light_component.h"

namespace pathos {

	static constexpr int32 MAX_DIRECTIONAL_LIGHTS = 4;
	static constexpr int32 MAX_POINT_LIGHTS = 8;

	struct UBO_Translucency {
		static constexpr uint32 BINDING_POINT = 1;

		matrix4   mvMatrix;
		matrix4   mvpMatrix;
		matrix3x4 mvMatrix3x3;
		vector4   albedo;
		vector4   metallic_roughness;
		vector4   transmittance_opacity;
	};

	struct UBO_LightInfo {
		static constexpr uint32 BINDING_POINT = 2;

		vector4i              numLightSources; // (directional, point, ?, ?)
		DirectionalLightProxy directionalLights[MAX_DIRECTIONAL_LIGHTS];
		PointLightProxy       pointLights[MAX_POINT_LIGHTS];
	};

	class TranslucencyVS : public ShaderStage {
	public:
		TranslucencyVS() : ShaderStage(GL_VERTEX_SHADER, "TranslucencyVS") {
			setFilepath("translucency_vs.glsl");
		}
	};
	class TranslucencyFS : public ShaderStage {
	public:
		TranslucencyFS() : ShaderStage(GL_FRAGMENT_SHADER, "TranslucencyFS") {
			setFilepath("translucency_fs.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_Translucency, TranslucencyVS, TranslucencyFS);
}

namespace pathos {

	TranslucencyRendering::TranslucencyRendering()
	{
		ubo.init<UBO_Translucency>();
		uboLight.init<UBO_LightInfo>();
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

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_Translucency);
		cmdList.useProgram(program.getGLName());

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
			uboLight.update(cmdList, UBO_LightInfo::BINDING_POINT, &uboData);
		}

		// #todo-translucency: drawcall
		uint8 materialID = (uint8)MATERIAL_ID::TRANSLUCENT_SOLID_COLOR;
		const auto& meshBatches = scene->proxyList_staticMesh[materialID];
		for (const StaticMeshProxy* meshBatch : meshBatches) {
			const matrix4& modelMatrix = meshBatch->modelMatrix;
			TranslucentColorMaterial* material = static_cast<TranslucentColorMaterial*>(meshBatch->material);
			MeshGeometry* geometry = meshBatch->geometry;

			UBO_Translucency uboData;
			uboData.mvMatrix              = camera->getViewMatrix() * modelMatrix;
			uboData.mvpMatrix             = camera->getViewProjectionMatrix() * modelMatrix;
			uboData.mvMatrix3x3           = matrix3x4(uboData.mvMatrix);
			uboData.albedo                = vector4(material->getAlbedo(), 0.0f);
			uboData.metallic_roughness.x  = material->getMetallic();
			uboData.metallic_roughness.y  = material->getRoughness();
			uboData.transmittance_opacity = vector4(material->getTransmittance(), material->getOpacity());
			ubo.update(cmdList, UBO_Translucency::BINDING_POINT, &uboData);

			geometry->activate_position_uv_normal_tangent_bitangent(cmdList);
			geometry->activateIndexBuffer(cmdList);
			geometry->drawPrimitive(cmdList);
		}

		// restore renderstates
		{
			cmdList.disable(GL_BLEND);
			cmdList.disable(GL_DEPTH_TEST);
		}
	}

}
