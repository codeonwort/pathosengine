#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"

namespace pathos {

	class OpenGLDevice;
	class MeshGeometry;

	enum class EIrradianceMapEncoding : uint32 { Cubemap, OctahedralNormalVector };

	struct IrradianceMapBakeDesc {
		EIrradianceMapEncoding encoding;
		GLuint renderTarget; // Cubemap or 2D atlas texture to render the irradiance map.
		GLuint depthTarget; // 2D atlas to render probe linear depth.
		uint32 viewportSize;
		vector2ui viewportOffset = vector2ui(0, 0); // Only meaningful for ONV.
	};

	// Utility for Image Based Lighting.
	class LightProbeBaker {

	public:
		static LightProbeBaker& get();
		static void static_initializeResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
		static void static_destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);

	public:
		LightProbeBaker();

		void initializeResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
		void destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);

		/// <summary>
		/// Generate irradiance cubemap from radiance capture cubemap.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="inputRadianceCubemap">Input radiance cubemap</param>
		/// <param name="inputDepthCubemap">Input depth cubemap</param>
		/// <param name="bakeDesc">Baking options</param>
		void bakeDiffuseIBL_renderThread(
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
		void bakeSpecularIBL_renderThread(
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
		void projectPanoramaToCubemap_renderThread(
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
		void bakeSkyIrradianceMap_renderThread(
			RenderCommandList& cmdList,
			GLuint inputSkyCubemap,
			GLuint targetCubemap,
			uint32 targetSize);

		/// <summary>
		/// New implementation for reflection probe filtering, but only support 128-sized cubemaps.
		/// Therefore sky specular IBLs still use old impl.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="srcCubemap">Radiance-captured cubemap. Should have size of 128 and mip count of 7.</param>
		/// <param name="dstCubemap">Cubemap that will store the filtering result. Should have size of 128 and mip count of 7.</param>
		void bakeReflectionProbe_renderThread(RenderCommandList& cmdList, GLuint srcCubemap, GLuint dstCubemap);

		// -----------------------------------------------------------------------

		// Default BRDF integration map of 512 size
		GLuint getBRDFIntegrationMap_512() { return internal_BRDFIntegrationMap; }

		GLuint bakeBRDFIntegrationMap_renderThread(uint32 size, RenderCommandList& cmdList);

	private:
		GLuint dummyVAO;
		GLuint dummyFBO; // Dummy FBO for render to a 2D texture or one face of a cubemap
		GLuint dummyFBO_2color; // Dummy FBO for two color attachments
		MeshGeometry* fullscreenQuad;
		MeshGeometry* dummyCube;
		matrix4 cubeTransforms[6];

		GLuint internal_BRDFIntegrationMap;

	};

}
