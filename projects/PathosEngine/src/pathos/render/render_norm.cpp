#include "pathos/render/render_norm.h"
#include "pathos/shader/shader.h"

namespace pathos {

	NormalRenderer::NormalRenderer(float normLen): normalLength(normLen) {
		VertexShaderSource* vs = new VertexShaderSource;
		GeometryShaderSource* gs = new GeometryShaderSource("triangles", "line_strip", 2);
		FragmentShaderSource* fs = new FragmentShaderSource;
		vector<ShaderSource*> shaders = { vs, gs, fs };

		// vs
		vs->setUseNormal(true);
		vs->outVar("vec4", "normalAdded");
		vs->uniform("float", "normalLength");
		vs->mainCode("vs_out.normalAdded = mvpTransform * vec4(position + normal * normalLength, 1);");
		//Shader* v = new Shader(GL_VERTEX_SHADER);
		//v->setSource(vs->getCode());

		// gs
		gs->inVar("vec3", "normal");
		gs->inVar("vec4", "normalAdded");
		gs->mainCode("gl_Position = gl_in[0].gl_Position;");
		gs->mainCode("EmitVertex();");
		gs->mainCode("gl_Position = gs_in[0].normalAdded;");
		gs->mainCode("EmitVertex();");
		gs->mainCode("EndPrimitive();");
		//Shader* g = new Shader(GL_GEOMETRY_SHADER);
		//g->loadSource("normal_visualizer.glsl");

		// fs
		fs->interfaceBlockName("GS_OUT");
		fs->outVar("vec4", "color");
		fs->mainCode("color = vec4(1, 0, 0, 1);");
		//Shader* f = new Shader(GL_FRAGMENT_SHADER);
		//f->setSource(fs->getCode());

		program = createProgram(shaders);

		delete vs;
		delete gs;
		delete fs;
	}

	void NormalRenderer::render(Mesh* mesh, Camera* camera) {
		glm::mat4 model = mesh->getTransform().getMatrix();
		glm::mat4 mvp = camera->getViewProjectionMatrix() * model;

		glUseProgram(program);
		glUniform1f(glGetUniformLocation(program, "normalLength"), normalLength);
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, GL_FALSE, &mvp[0][0]);

		//glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		Geometries geoms = mesh->getGeometries();
		for (auto i = 0u; i < geoms.size(); i++){
			geoms[i]->activate_position_normal();
			geoms[i]->activateIndexBuffer();
			geoms[i]->draw();
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glEnable(GL_CULL_FACE);

		glUseProgram(0);
	}

}
