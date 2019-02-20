// Volumetric light scattering a.k.a. god ray

#pragma once

#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/mesh/mesh.h"
#include "gl_core.h"

namespace pathos {

	// TODO: need depth buffer
	class GodRay {

		static constexpr unsigned int GOD_RAY_SOURCE = 0;
		static constexpr unsigned int GOD_RAY_RESULT = 1;

	public:
		GodRay(unsigned int width, unsigned int height);
		~GodRay();

		void initialize(unsigned int width, unsigned int height);
		void createFBO();
		void createShaders();

		void render(Scene* scene, Camera* camera);
		void renderSilhouette(Camera* camera, Mesh* mesh, GLfloat* color);

		inline GLuint getTexture() { return textures[GOD_RAY_RESULT]; }

	private:
		unsigned int width, height;
		GLuint fbo[2], textures[2];

		GLuint program_silhouette;
		GLint uniform_mvp;
		GLint uniform_color;
		
		GLuint program_godRay;
		GLint uniform_lightPos;

		GLuint vao_dummy;

	};

}