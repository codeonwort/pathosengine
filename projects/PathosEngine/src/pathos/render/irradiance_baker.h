#pragma once

#include "gl_core.h"

namespace pathos {

	class IrradianceBaker {

	public:
		// Given a 2D texture, creates and returns a new cubemap texture
		static GLuint bakeCubemap(GLuint equirectangularMap, uint32 size, GLuint oldCubemap = 0);

		static GLuint equirectangularToCubemap;
		static GLuint dummyVAO;
		static GLuint dummyFBO;
		static class CubeGeometry* dummyCube;

	};

}
