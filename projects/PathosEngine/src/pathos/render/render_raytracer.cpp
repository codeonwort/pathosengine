#include "render_raytracer.h"
#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"

#include "pathos/shader/shader.h"

namespace pathos {

	Raytracer::Raytracer(int32_t screenWidth_, int32_t screenHeight_)
		: screenWidth(screenWidth_)
		, screenHeight(screenHeight_)
		, program_raytracer(0)
	{
		createShaders();
	}

	Raytracer::~Raytracer() {
		destroyShaders();
	}

	void Raytracer::createFBO() {
		glGenFramebuffers(1, &fbo);
	}

	void Raytracer::destroyFBO() {
		glDeleteFramebuffers(1, &fbo);
	}

	void Raytracer::createShaders() {
		// TODO
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);

		vs.loadSource("raytracer_prepare_vs.glsl");
		fs.loadSource("raytracer_prepare_fs.glsl");
		program_prepare = pathos::createProgram(vs, fs, "Raytracer_Prepare");

		vs.loadSource("fullscreen_quad.glsl");
		fs.loadSource("raytracer.glsl");
		program_raytracer = pathos::createProgram(vs, fs, "Raytracer_Execution");
	}

	void Raytracer::destroyShaders() {
		// TODO
		glDeleteProgram(program_raytracer);
	}

	void Raytracer::render(Scene* scene, Camera* camera) {
		//
	}

}