#pragma once

#include "gl_core.h"

namespace pathos {

	class IrradianceBaker {

	public:
		// Given a 2D texture, creates and returns a new cubemap texture
		static GLuint bakeCubemap(GLuint equirectangularMap, uint32 size, const char* debugName = nullptr);

		// cubemap : a texture returned by bakeCubemap()
		// size    : size of the cubemap that will be returned
		static GLuint bakeIrradianceMap(GLuint cubemap, uint32 size, bool autoDestroyCubemap);

		static void bakePrefilteredEnvMap(GLuint cubemap, uint32 size, GLuint& outEnvMap, uint32& outMipLevels);

		// Default BRDF integration map of 512 size
		static GLuint getBRDFIntegrationMap_512() { return internal_BRDFIntegrationMap; }

		static GLuint bakeBRDFIntegrationMap_renderThread(uint32 size);

		static void internal_createIrradianceBakerResources(class OpenGLDevice* renderDevice);
		static void internal_destroyIrradianceBakerResources(class OpenGLDevice* renderDevice);

	private:
		static GLuint equirectangularToCubemap;
		static GLuint diffuseIrradianceShader;
		static GLuint prefilterEnvMapShader;
		static GLuint BRDFIntegrationMapShader;

		static GLuint dummyVAO;
		static GLuint dummyFBO; // Dummy FBO for render to a 2D texture or one face of a cubemap
		static class PlaneGeometry* fullscreenQuad;
		static class CubeGeometry* dummyCube;
		static glm::mat4 cubeTransforms[6];

		static GLuint internal_BRDFIntegrationMap;

	};

}
