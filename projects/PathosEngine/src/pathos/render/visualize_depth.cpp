#include "visualize_depth.h"
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
		program = pathos::createProgram(vs, fs);

		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_VisualizeDepth), (void*)0, GL_DYNAMIC_DRAW);
	}

	VisualizeDepth::~VisualizeDepth()
	{
		glDeleteProgram(program);
		glDeleteBuffers(1, &ubo);
	}

	void VisualizeDepth::render(Scene* scene, Camera* camera)
	{
		SCOPED_DRAW_EVENT(VisualizeDepth);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		UBO_VisualizeDepth uboData;
		uboData.zRange.x = camera->getZNear();
		uboData.zRange.y = camera->getZFar();

		const glm::mat4 viewProj = camera->getViewProjectionMatrix();

		glUseProgram(program);

		for (Mesh* mesh : scene->meshes) {
			const glm::mat4 model = mesh->getTransform().getMatrix();
			uboData.mvp = viewProj * model;
			glNamedBufferSubData(ubo, 0, sizeof(UBO_VisualizeDepth), &uboData);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

			for (MeshGeometry* geometry : mesh->getGeometries()) {
				geometry->activate_position();
				geometry->activateIndexBuffer();
				geometry->draw();
			}
		}
	}

}
