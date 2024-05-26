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
		/// <summary>
		/// Generate irradiance cubemap from radiance capture cubemap.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="inputRadianceCubemap">Input radiance cubemap</param>
		/// <param name="inputDepthCubemap">Input depth cubemap</param>
		/// <param name="bakeDesc">Baking options</param>
		static void bakeDiffuseIBL_renderThread(
			RenderCommandList& cmdList,
			GLuint inputRadianceCubemap,
			GLuint inputDepthCubemap,
			const IrradianceMapBakeDesc& bakeDesc);

		/// <summary>
		/// Render specular IBL cubemap from radiance capture cubemap.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="inputTexture">Input radiance cubemap</param>
		/// <param name="outputTextureSize">Output cubemap size</param>
		/// <param name="numMips">The number of mips to render</param>
		/// <param name="outputTexture">Output cubemap</param>
		static void bakeSpecularIBL_renderThread(
			RenderCommandList& cmdList,
			GLuint inputTexture,
			uint32 outputTextureSize,
			uint32 numMips,
			GLuint outputTexture);

		/// <summary>
		/// Render the given equirectangular (panorama) texture to the cubemap texture.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="inputTexture">Input panorama texture2D</param>
		/// <param name="outputTexture">Output textureCube</param>
		/// <param name="outputTextureSize">Output texture size</param>
		static void projectPanoramaToCubemap_renderThread(
			RenderCommandList& cmdList,
			GLuint inputTexture,
			GLuint outputTexture,
			uint32 outputTextureSize);

		/// <summary>
		/// Similar to bakeDiffuseIBL_renderThread(), but only for sky lighting.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="inputSkyCubemap">Sky cubemap from which irradiance will be integrated</param>
		/// <param name="targetCubemap">Target cubemap to store sky irradiance map</param>
		/// <param name="targetSize">The size of target cubemap</param>
		static void bakeSkyIrradianceMap_renderThread(
			RenderCommandList& cmdList,
			GLuint inputSkyCubemap,
			GLuint targetCubemap,
			uint32 targetSize);

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
