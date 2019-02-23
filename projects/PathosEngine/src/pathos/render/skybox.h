#pragma once

#include "sky.h"
#include "pathos/mesh/geometry_primitive.h"
#include "gl_core.h"

namespace pathos {

	class Skybox : public SkyRendering {

	public:
		Skybox(GLuint textureID);
		~Skybox();

		void render(const Scene* scene, const Camera* camera) override;

	private:
		void createShader();

		GLuint program;
		GLuint textureID;
		GLint uniform_transform = -1;

		CubeGeometry* cube;

	};

}
