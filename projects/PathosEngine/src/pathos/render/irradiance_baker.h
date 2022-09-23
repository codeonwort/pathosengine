#pragma once

#include "gl_core.h"

namespace pathos {

	class IrradianceBaker {

	public:
		// Given a 2D texture, creates and returns a new cubemap texture
		static GLuint bakeCubemap(GLuint equirectangularMap, uint32 size, const char* debugName = nullptr);

		// cubemap : a texture returned by bakeCubemap()
		// size    : size of the cubemap that will be returned
		static GLuint bakeIrradianceMap(GLuint cubemap, uint32 size, bool autoDestroyCubemap, const char* debugName = nullptr);

		static void bakePrefilteredEnvMap(GLuint cubemap, uint32 size, GLuint& outEnvMap, uint32& outMipLevels, const char* debugName = nullptr);

		// Default BRDF integration map of 512 size
		static GLuint getBRDFIntegrationMap_512() { return internal_BRDFIntegrationMap; }

		static GLuint bakeBRDFIntegrationMap_renderThread(uint32 size, RenderCommandList& cmdList);

		static void internal_createIrradianceBakerResources(class OpenGLDevice* renderDevice, RenderCommandList& cmdList);
		static void internal_destroyIrradianceBakerResources(class OpenGLDevice* renderDevice, RenderCommandList& cmdList);

	private:
		static GLuint dummyVAO;
		static GLuint dummyFBO; // Dummy FBO for render to a 2D texture or one face of a cubemap
		static class PlaneGeometry* fullscreenQuad;
		static class CubeGeometry* dummyCube;
		static glm::mat4 cubeTransforms[6];

		static GLuint internal_BRDFIntegrationMap;

	};

}
