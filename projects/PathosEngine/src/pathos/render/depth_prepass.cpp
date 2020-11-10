#include "depth_prepass.h"
#include "render_device.h"
#include "scene_render_targets.h"
#include "pathos/scene/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader_program.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/material/material.h"

namespace pathos {

	class DepthPrepassVS : public ShaderStage {
	public:
		DepthPrepassVS() : ShaderStage(GL_VERTEX_SHADER, "DepthPrepassVS")
		{
			addDefine("VERTEX_SHADER 1");
			setFilepath("depth_prepass.glsl");
		}
	};

	class DepthPrepassFS : public ShaderStage {
	public:
		DepthPrepassFS() : ShaderStage(GL_FRAGMENT_SHADER, "DepthPrepassFS")
		{
			addDefine("FRAGMENT_SHADER 1");
			setFilepath("depth_prepass.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_DepthPrepass, DepthPrepassVS, DepthPrepassFS);

}

namespace pathos {

	struct UBO_DepthPrepass {
		matrix4 mvpTransform;
	};

	void DepthPrepass::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_DepthPrepass");

		ubo.init<UBO_DepthPrepass>();
	}

	void DepthPrepass::destroyResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void DepthPrepass::renderPreDepth(RenderCommandList& cmdList, Scene* scene, Camera* camera)
	{
		SCOPED_DRAW_EVENT(DepthPrepass);
		
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		static const GLfloat zeroDepth[] = { 0.0f };

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DepthPrepass);
		cmdList.useProgram(program.getGLName());

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		// Reverse-Z
		cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		cmdList.depthFunc(GL_GREATER);
		cmdList.enable(GL_DEPTH_TEST);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);
		cmdList.clearNamedFramebufferfv(fbo, GL_DEPTH, 0, zeroDepth);

		for (uint8 i = 0; i < (uint8)(MATERIAL_ID::NUM_MATERIAL_IDS); ++i) {
			const auto& proxyList = scene->proxyList_staticMesh[i];
			for (StaticMeshProxy* proxy : proxyList) {
				if (proxy->material->getMaterialID() == MATERIAL_ID::TRANSLUCENT_SOLID_COLOR) {
					continue;
				}

				bool wireframe = proxy->material->getMaterialID() == MATERIAL_ID::WIREFRAME;
				if (wireframe) {
					cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}

				UBO_DepthPrepass uboData;
				uboData.mvpTransform = camera->getViewProjectionMatrix() * proxy->modelMatrix;
				ubo.update(cmdList, 1, &uboData);

				proxy->geometry->activate_position(cmdList);
				proxy->geometry->activateIndexBuffer(cmdList);
				proxy->geometry->drawPrimitive(cmdList);
				proxy->geometry->deactivate(cmdList);
				proxy->geometry->deactivateIndexBuffer(cmdList);

				if (wireframe) {
					cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
			}
		}
	}

}
