#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"

// #todo-light-probe: This file was written for sky IBL, several years ago.
// Need to refactor and rename all functions.

namespace pathos {

	enum class EIrradianceMapEncoding : uint32 { Cubemap, OctahedralNormalVector };

	struct IrradianceMapBakeDesc {
		EIrradianceMapEncoding encoding;
		GLuint renderTarget; // Cubemap or 2D texture to render the irradiance map.
		uint32 viewportSize;
		vector2ui viewportOffset = vector2ui(0, 0); // Only meaningful for ONV.
	};

	// RENAME: More like a IBLBaker.
	// REFACTOR: Need render thread versions that do not perform GPU flushes.
	class IrradianceBaker {

	public:
		// Create a cubemap equivalent of the given equirectangular 2D texture.
		// @param equirectangularMap : 2D input texture.
		// @param size               : Output cubemap size.
		// @param debugName          : Output cubemap's GL name.
		static GLuint projectToCubemap(GLuint equirectangularMap, uint32 size, const char* debugName = nullptr);

		// Generate irradiance cubemap from radiance capture cubemap.
		// @param inputCubemap : Radiance cubemap.
		// @param bakeDesc     : Irradiance map desc.
		static void bakeDiffuseIBL_renderThread(RenderCommandList& cmdList, GLuint inputCubemap, const IrradianceMapBakeDesc& bakeDesc);

		// Create a cubemap texture and bake irradiance to it. Flush render thread and GPU.
		// @param inputCubemap : Radiance cubemap from which irradiance will be integrated.
		// @param size         : size of the cubemap that will be returned
		static GLuint bakeIrradianceMap(GLuint inputCubemap, uint32 size, bool bAutoDestroyInputCubemap, const char* debugName = nullptr);

		// Render specular IBL to an existing cubemap.
		static void bakeSpecularIBL_renderThread(RenderCommandList& cmdList, GLuint inputTexture, uint32 textureSize, uint32 numMips, GLuint outputTexture);

		// Create a cubemap texture and bake specular IBL to it. Flush render thread and GPU.
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
