#pragma once

#include "sky.h"
#include "pathos/mesh/geometry_primitive.h"
#include "gl_core.h"

namespace pathos {

	class Skybox : public SkyRendering {

	public:
		Skybox(GLuint textureID);
		~Skybox();

		void setLOD(float inLOD);

		void render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera) override;

	private:
		void createShader();

		GLuint program;
		GLuint textureID;
		GLint uniform_transform = -1;
		GLint uniform_lod = -1;

		CubeGeometry* cube;
		float lod;

	};

}
