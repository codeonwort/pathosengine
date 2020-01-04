// Light shaft a.k.a. god ray

#pragma once

#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/mesh/mesh.h"
#include "pathos/shader/uniform_buffer.h"
#include "gl_core.h"

namespace pathos {

	// TODO: need depth buffer
	class GodRay final {

		static constexpr uint32 GOD_RAY_SOURCE = 0;
		static constexpr uint32 GOD_RAY_RESULT = 1;

	public:
		GodRay();
		~GodRay();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void createFBO(RenderCommandList& cmdList);
		void createShaders(RenderCommandList& cmdList);

		void renderGodRay(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* fullscreenQuad);

	private:
		void renderSilhouette(RenderCommandList& cmdList, Camera* camera, Mesh* mesh, GLfloat* color);

	private:
		bool destroyed = false;

		GLuint fbo[2] = { 0, 0 };

		GLuint program_silhouette = 0;
		GLint uniform_mvp;
		GLint uniform_color;
		
		GLuint program_godRay = 0;
		GLint uniform_lightPos;

		GLuint fboBlur1 = 0xffffffff;
		GLuint fboBlur2 = 0xffffffff;
		GLuint program_blur1 = 0;
		GLuint program_blur2 = 0;

		GLuint vao_dummy;

	};

}
