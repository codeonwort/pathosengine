// Volumetric light scattering a.k.a. god ray

#include "god_ray.h"
#include "glm/gtc/type_ptr.hpp"

#if defined(_DEBUG)
#include <iostream>
#include <cassert>
#endif

namespace pathos {

	GodRay::GodRay(unsigned int width, unsigned int height) {
		initialize(width, height);
	}

	GodRay::~GodRay() {
		glDeleteVertexArrays(1, &vao_dummy);
		glDeleteFramebuffers(2, fbo);
		glDeleteTextures(2, textures);
		glDeleteProgram(program_silhouette);
		glDeleteProgram(program_godRay);
	}

	void GodRay::initialize(unsigned int width_, unsigned int height_) {
		width = width_;
		height = height_;
		createFBO();
		createShaders();
		glGenVertexArrays(1, &vao_dummy);
	}

	void GodRay::createFBO() {
		// generate fbo and textures
		glGenFramebuffers(2, fbo);
		glGenTextures(2, textures);
		GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0 };

		// pass 1
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[GOD_RAY_SOURCE]);
		glBindTexture(GL_TEXTURE_2D, textures[GOD_RAY_SOURCE]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[GOD_RAY_SOURCE], 0);
		glDrawBuffers(1, drawBuffers);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#if defined(_DEBUG)
			std::cerr << "Cannot create a framebuffer for god ray source" << std::endl;
#endif
			assert(0);
		}

		// pass 2
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[GOD_RAY_RESULT]);
		glBindTexture(GL_TEXTURE_2D, textures[GOD_RAY_RESULT]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[GOD_RAY_RESULT], 0);
		glDrawBuffers(1, drawBuffers);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#if defined(_DEBUG)
			std::cerr << "Cannot create a framebuffer for god ray result" << std::endl;
#endif
			assert(0);
		}

		// unbind
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GodRay::createShaders() {
		std::string vshader, fshader;

		/////////////////////////////////////////////////////////////////////////////////
		// program - silhouette
		vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 mvpTransform;

void main() {
	gl_Position = mvpTransform * vec4(position, 1.0);
}
)";

		fshader = R"(
#version 430 core

uniform vec3 color;

out vec4 out_color;

void main() {
	out_color = vec4(color, 1.0);
}
)";

		program_silhouette = pathos::createProgram(vshader, fshader);
		uniform_mvp = glGetUniformLocation(program_silhouette, "mvpTransform");
		uniform_color = glGetUniformLocation(program_silhouette, "color");
		assert(uniform_mvp != -1 && uniform_color != -1);

		/////////////////////////////////////////////////////////////////////////////////
		// program - god ray
		vshader = R"(
#version 430 core

out vec2 uv;

void main() {
	const vec3[4] vertices = vec3[4](vec3(-1,-1,1), vec3(1,-1,1), vec3(-1,1,1), vec3(1,1,1));
	const vec2[4] uvs = vec2[4](vec2(0,0), vec2(1,0), vec2(0,1), vec2(1,1));

	uv = uvs[gl_VertexID];
	gl_Position = vec4(vertices[gl_VertexID], 1.0);
}
)";

		fshader = R"(
#version 430 core

in vec2 uv;

layout (binding = 0) uniform sampler2D src;

uniform vec2 lightPos = vec2(0.5, 0.5);

const float alphaDecay = 0.95;
const float density = 1.1;
const int NUM_SAMPLES = 100;

out vec4 out_color;

void main() {
	vec2 delta = (uv - lightPos) * (1.0 / (density * NUM_SAMPLES));
	vec2 pos = uv;
	vec4 result = vec4(0.0);
	float alpha = 1.0;

	for(int i = 0; i < NUM_SAMPLES; ++i) {
		pos -= delta;
		result += alpha * texture2D(src, pos);
		alpha *= alphaDecay;
	}
	out_color = result;
}
)";
		
		program_godRay = pathos::createProgram(vshader, fshader);
		uniform_lightPos = glGetUniformLocation(program_godRay, "lightPos");
		assert(uniform_lightPos != -1);
	}

	void GodRay::render(Scene* scene, Camera* camera) {
		// bind
		static GLfloat transparent_black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static GLfloat opaque_black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static GLfloat opaque_white[] = { 1.0f, 0.5f, 0.0f, 1.0f };

		glBindFramebuffer(GL_FRAMEBUFFER, fbo[GOD_RAY_SOURCE]);
		glClearBufferfv(GL_COLOR, 0, transparent_black);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[GOD_RAY_RESULT]);
		glClearBufferfv(GL_COLOR, 0, transparent_black);

		// special case: no light source
		if (scene->godRaySource == nullptr) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return;
		}

		// render
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[GOD_RAY_SOURCE]);
		glUseProgram(program_silhouette);
		renderSilhouette(camera, scene->godRaySource, opaque_white);
		for (Mesh* mesh : scene->meshes) {
			if (mesh == scene->godRaySource) continue;
			renderSilhouette(camera, mesh, opaque_black);
		}

		// light scattering pass
		auto lightPos = scene->godRaySource->getTransform().getPosition();
		const auto lightMVP = camera->getViewProjectionMatrix();
		auto lightPos_homo = lightMVP * glm::vec4(lightPos, 1.0f);
		lightPos = glm::vec3(lightPos_homo) / lightPos_homo.w;
		GLfloat lightPos_2d[2] = { (lightPos.x + 1.0f) / 2.0f, (lightPos.y + 1.0f) / 2.0f };

		glBindVertexArray(vao_dummy);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo[GOD_RAY_RESULT]);
		glUseProgram(program_godRay);
		glBindTextureUnit(0, textures[GOD_RAY_SOURCE]);
		glUniform2fv(uniform_lightPos, 1, lightPos_2d);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // gl error?

		// unbind
		glBindVertexArray(0);
		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GodRay::renderSilhouette(Camera* camera, Mesh* mesh, GLfloat* color) {
		const auto modelMatrix = mesh->getTransform().getMatrix();
		const auto geoms = mesh->getGeometries();
		const auto materials = mesh->getMaterials();
		glUniform3fv(uniform_color, 1, color);
		glUniformMatrix4fv(uniform_mvp, 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));
		for (auto i = 0u; i < geoms.size(); ++i) {
			MeshGeometry* G = geoms[i];
			MeshMaterial* M = materials[i];

			bool wireframe = M != nullptr && M->getMaterialID() == MATERIAL_ID::WIREFRAME;
			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}

			G->activate_position();
			G->activateIndexBuffer();
			G->draw();
			G->deactivate();
			G->deactivateIndexBuffer();

			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
	}

}