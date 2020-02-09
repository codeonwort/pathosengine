#pragma once

#include "gl_core.h"

namespace pathos {

	class IrradianceBaker {

	public:
		// Given a 2D texture, creates and returns a new cubemap texture
		static GLuint bakeCubemap(GLuint equirectangularMap, uint32 size);

		// cubemap : a texture returned by bakeCubemap()
		// size    : size of the cubemap that will be returned
		static GLuint bakeIrradianceMap(GLuint cubemap, uint32 size, bool autoDestroyCubemap);

		static GLuint equirectangularToCubemap;
		static GLuint diffuseIrradianceShader;
		static GLuint dummyVAO; // #todo-deprecated: delete this
		static GLuint dummyFBO;
		static class CubeGeometry* dummyCube;

	};

}
