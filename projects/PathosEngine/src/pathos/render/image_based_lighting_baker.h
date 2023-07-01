#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"

namespace pathos {

	enum class EIrradianceMapEncoding : uint32 { Cubemap, OctahedralNormalVector };

	struct IrradianceMapBakeDesc {
		EIrradianceMapEncoding encoding;
		GLuint renderTarget; // Cubemap or 2D atlas texture to render the irradiance map.
		GLuint depthTarget; // 2D atlas to render probe linear depth.
		uint32 viewportSize;
		vector2ui viewportOffset = vector2ui(0, 0); // Only meaningful for ONV.
	};

	// Utility for Image Based Lighting.
	class ImageBasedLightingBaker {

	public:
		// Generate irradiance cubemap from radiance capture cubemap.
		// @param inputRadianceCubemap : Radiance cubemap.
		// @param inputDepthCubemap    : Depth cubemap.
		// @param bakeDesc             : Irradiance map desc.
		static void bakeDiffuseIBL_renderThread(
			RenderCommandList& cmdList,
			GLuint inputRadianceCubemap,
			GLuint inputDepthCubemap,
			const IrradianceMapBakeDesc& bakeDesc);

		// Render specular IBL to an existing cubemap.
		// @param inputTexture  : Radiance cubemap.
		// @param textureSize   : Width and height of outputTexture.
		// @param numMips       : Mip count of outputTexture.
		// @param outputTexture : Cubemap render target. Should be a valid GL texture.
		static void bakeSpecularIBL_renderThread(
			RenderCommandList& cmdList,
			GLuint inputTexture,
			uint32 outputTextureSize,
			uint32 numMips,
			GLuint outputTexture);

		// -----------------------------------------------------------------------
		// API for sky IBL but obsolete.
		// - Assumes sky is static.
		// - Flushes render thread and GPU.

		// Create a cubemap equivalent of the given equirectangular 2D texture.
		// NOTE: Flush render thread and GPU.
		// @param equirectangularMap : 2D input texture.
		// @param size               : Output cubemap size.
		// @param debugName          : Output cubemap's GL name.
		static GLuint projectPanoramaToCubemap(
			GLuint equirectangularMap,
			uint32 size,
			const char* debugName = nullptr);

		// Render the given equirectangular (panorama) texture to the cubemap.
		// @param cmdList           : Render command list.
		// @param inputTexture      : Input panorama texture2D.
		// @param outputTexture     : Output textureCube.
		// @param outputTextureSize : Output texture size.
		static void projectPanoramaToCubemap_renderThread(
			RenderCommandList& cmdList,
			GLuint inputTexture,
			GLuint outputTexture,
			uint32 outputTextureSize);

		// Create a cubemap texture and bake irradiance to it.
		// NOTE: Flush render thread and GPU.
		// @param inputCubemap             : Radiance cubemap from which irradiance will be integrated.
		// @param size                     : Size of the cubemap that will be returned.
		// @param bAutoDestroyInputCubemap : Destroy input cubemap texture after irradiance map is generated.
		// @param debugName                : GL debug name of the returned texture.
		// @return Name of GL texture that represents irradiance map.
		static GLuint bakeSkyIrradianceMap(
			GLuint inputCubemap,
			uint32 size,
			bool bAutoDestroyInputCubemap,
			const char* debugName = nullptr);

		static void bakeSkyIrradianceMap_renderThread(
			RenderCommandList& cmdList,
			GLuint inputSkyCubemap,
			GLuint targetCubemap,
			uint32 targetSize);

		// Create a cubemap texture and bake specular IBL to it.
		// NOTE: Flush render thread and GPU.
		static void bakeSkyPrefilteredEnvMap(
			GLuint cubemap,
			uint32 targetSize,
			GLuint& outEnvMap,
			uint32& outMipLevels,
			const char* debugName = nullptr);

		// -----------------------------------------------------------------------

		// Default BRDF integration map of 512 size
		static GLuint getBRDFIntegrationMap_512() { return internal_BRDFIntegrationMap; }

		static GLuint bakeBRDFIntegrationMap_renderThread(uint32 size, RenderCommandList& cmdList);

		static void internal_createIrradianceBakerResources(class OpenGLDevice* renderDevice, RenderCommandList& cmdList);
		static void internal_destroyIrradianceBakerResources(class OpenGLDevice* renderDevice, RenderCommandList& cmdList);

	private:
		static GLuint dummyVAO;
		static GLuint dummyFBO; // Dummy FBO for render to a 2D texture or one face of a cubemap
		static GLuint dummyFBO_2color; // Dummy FBO for two color attachments
		static class MeshGeometry* fullscreenQuad;
		static class MeshGeometry* dummyCube;
		static matrix4 cubeTransforms[6];

		static GLuint internal_BRDFIntegrationMap;

	};

}
