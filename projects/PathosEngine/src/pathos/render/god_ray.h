// Volumetric light scattering a.k.a. god ray

#pragma once

#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/mesh/mesh.h"
#include "gl_core.h"

namespace pathos {

	// TODO: need depth buffer
	class GodRay {

		static constexpr uint32 GOD_RAY_SOURCE = 0;
		static constexpr uint32 GOD_RAY_RESULT = 1;

	public:
		GodRay();
		~GodRay();

		void initialize(RenderCommandList& cmdList);
		void destroy(RenderCommandList& cmdList);

		void createFBO(RenderCommandList& cmdList);
		void createShaders(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, Scene* scene, Camera* camera);
		void renderSilhouette(RenderCommandList& cmdList, Camera* camera, Mesh* mesh, GLfloat* color);

	private:
		bool destroyed = false;

		GLuint fbo[2] = { 0, 0 };

		GLuint program_silhouette;
		GLint uniform_mvp;
		GLint uniform_color;
		
		GLuint program_godRay;
		GLint uniform_lightPos;

		GLuint vao_dummy;

	};

}
