#include "translucency_rendering.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/shader/shader.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"

namespace pathos {

	struct UBO_Translucency {
		matrix4   mvMatrix;
		matrix4   mvpMatrix;
		matrix3x4 mvMatrix3x3;
		vector4   albedo;
		vector4   metallic_roughness;
		vector4   transmittance_opacity;
	};

	TranslucencyRendering::TranslucencyRendering()
	{
		Shader vs(GL_VERTEX_SHADER, "Translucency_VS");
		Shader fs(GL_FRAGMENT_SHADER, "Translucency_FS");
		vs.loadSource("translucency_vs.glsl");
		fs.loadSource("translucency_fs.glsl");

		shaderProgram = pathos::createProgram(vs, fs, "Translucency");

		ubo.init<UBO_Translucency>();
	}

	TranslucencyRendering::~TranslucencyRendering()
	{
		glDeleteProgram(shaderProgram);
	}

	void TranslucencyRendering::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		cmdList.flushAllCommands();
	}

	void TranslucencyRendering::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void TranslucencyRendering::renderTranslucency(RenderCommandList& cmdList, const Camera* camera, const std::vector<StaticMeshProxy*>& meshBatches)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		// renderstates for translucency
		{
			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

			cmdList.enable(GL_DEPTH_TEST);
			cmdList.depthFunc(GL_GREATER); 
		}

		cmdList.useProgram(shaderProgram);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		// #todo-translucency: Sort by z
		{
		}

		// #todo-translucency: drawcall
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
			ubo.update(cmdList, 1, &uboData);

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
