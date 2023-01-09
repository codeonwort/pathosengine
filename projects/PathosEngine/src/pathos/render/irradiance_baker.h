#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"

#include "badger/types/int_types.h"
#include "badger/types/matrix_types.h"

// #todo-light-probe: This file was written for sky IBL, several years ago.
// Need to refactor and rename all functions.

namespace pathos {

	// RENAME: More like a IBLBaker.
	// REFACTOR: Need render thread versions that do not perform GPU flushes.
	class IrradianceBaker {

	public:
		// Given a 2D texture, creates and returns a new cubemap texture
		static GLuint bakeCubemap(GLuint equirectangularMap, uint32 size, const char* debugName = nullptr);

		// Generate irradiance cubemap from radiance capture cubemap.
		// @param inputTexture  : rgba16f cubemap.
		// @param textureSize   : Side length of cubemaps.
		// @param outputTexture : rgb16f cubemap.
		static void bakeDiffuseIBL_renderThread(RenderCommandList& cmdList, GLuint inputTexture, uint32 textureSize, GLuint outputTexture);

		// cubemap : a texture returned by bakeCubemap()
		// size    : size of the cubemap that will be returned
		static GLuint bakeIrradianceMap(GLuint cubemap, uint32 size, bool autoDestroyCubemap, const char* debugName = nullptr);

		// Render specular IBL to an existing cubemap.
		static void bakeSpecularIBL_renderThread(RenderCommandList& cmdList, GLuint inputTexture, uint32 textureSize, uint32 numMips, GLuint outputTexture);

		// Create the output texture and render specular IBL to it. Flushes GPU.
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
		static matrix4 cubeTransforms[6];

		static GLuint internal_BRDFIntegrationMap;

	};

}
