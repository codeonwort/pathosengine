#pragma once

#include "GL/gl_core_4_3.h"
#include <stdint.h>

namespace pathos {

	class Raytracer {

	public:
		Raytracer(int32_t screenWidth, int32_t screenHeight);
		virtual ~Raytracer();

		Raytracer(const Raytracer&) = delete;
		void operator=(const Raytracer&) = delete;

		void render(class Scene*, class Camera*);

	protected:
		void createFBO();
		void destroyFBO();

		void createShaders();
		void destroyShaders();

		int32_t screenWidth;
		int32_t screenHeight;

		GLuint program_prepare;
		GLuint program_raytracer;

		GLuint fbo;
		GLuint fbo_attachments[1];

	};

}