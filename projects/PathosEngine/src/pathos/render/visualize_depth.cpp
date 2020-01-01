#include "visualize_depth.h"
#include "scene_render_targets.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry.h"
#include "pathos/shader/shader.h"

namespace pathos {

	struct UBO_VisualizeDepth {
		glm::mat4 mvp;
		glm::vec4 zRange;
	};

	VisualizeDepth::VisualizeDepth()
	{
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("visualize_depth.vs.glsl");
		fs.loadSource("visualize_depth.fs.glsl");
		program = pathos::createProgram(vs, fs, "VisualizeDepth");

		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_VisualizeDepth), (void*)0, GL_DYNAMIC_DRAW);
	}

	VisualizeDepth::~VisualizeDepth()
	{
		glDeleteProgram(program);
		glDeleteBuffers(1, &ubo);
	}

	void VisualizeDepth::render(RenderCommandList& cmdList, Scene* scene, Camera* camera)
	{
		SCOPED_DRAW_EVENT(VisualizeDepth);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		cmdList.bindFramebuffer(GL_FRAMEBUFFER, 0);
		cmdList.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthFunc(GL_GREATER);

		UBO_VisualizeDepth uboData;
		uboData.zRange.x = camera->getZNear();
		uboData.zRange.y = camera->getZFar();

		const glm::mat4 viewProj = camera->getViewProjectionMatrix();

		cmdList.useProgram(program);

		for (Mesh* mesh : scene->meshes) {
			const glm::mat4 model = mesh->getTransform().getMatrix();
			uboData.mvp = viewProj * model;
			cmdList.namedBufferSubData(ubo, 0, sizeof(UBO_VisualizeDepth), &uboData);
			cmdList.bindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

			for (MeshGeometry* geometry : mesh->getGeometries()) {
				geometry->activate_position(cmdList);
				geometry->activateIndexBuffer(cmdList);
				geometry->drawPrimitive(cmdList);
			}
		}
	}

}
