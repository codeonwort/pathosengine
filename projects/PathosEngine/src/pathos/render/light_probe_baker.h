#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"

namespace pathos {

	class OpenGLDevice;
	class MeshGeometry;
	class Texture;
	class Buffer;

	enum class EIrradianceMapEncoding : uint32 { Cubemap, OctahedralNormalVector };

	struct IrradianceMapBakeDesc {
		EIrradianceMapEncoding encoding;
		GLuint renderTarget; // Cubemap or 2D atlas texture to render the irradiance map.
		GLuint depthTarget; // 2D atlas to render probe linear depth.
		uint32 viewportSize;
		vector2ui viewportOffset = vector2ui(0, 0); // Only meaningful for ONV.
	};

	// Utility for light probe based global illumination.
	class LightProbeBaker {

	public:
		static LightProbeBaker& get();
		static void static_initializeResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
		static void static_destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);

	public:
		LightProbeBaker();

		void initializeResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
		void destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);

		// New SH methods for diffuse irradiance baking.
		void bakeSkyDiffuseSH_renderThread(RenderCommandList& cmdList, Texture* inCubemap, Buffer* outSH);
		void bakeLightProbeSH_renderThread(RenderCommandList& cmdList, Texture* inColorCubemap, Texture* inDepthCubemap, Buffer* outSH, uint32 shIndex);
		void bakeOctahedralDepthAtlas_renderThread(RenderCommandList& cmdList, Texture* inDepthCubemap, GLuint depthAtlasTexture, const vector4ui& atlasTileCoordAndSize);

		// #todo-light-probe: Use Texture* instead of GLuint. Assert cubemap size.
		/// <summary>
		/// New implementation for reflection probe filtering, but only support 128-sized cubemaps.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="srcCubemap">Radiance-captured cubemap. Should have size of 128 and mip count of 7.</param>
		/// <param name="dstCubemap">Cubemap that will store the filtering result. Should have size of 128 and mip count of 7.</param>
		void bakeReflectionProbe_renderThread(RenderCommandList& cmdList, GLuint srcCubemap, GLuint dstCubemap);

		/// Default BRDF integration map of size 512.
		GLuint getBRDFIntegrationMap_512() { return bdfIntegrationMap; }

		/// Bake BRDF integration map. It's enough to call only once.
		GLuint bakeBRDFIntegrationMap_renderThread(RenderCommandList& cmdList, uint32 size);

	// Cubemap utils.
	public:
		/// <summary>
		/// Copy between cubemaps. input texture's target mip and output texture's target mip should have the same size.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="input">Input cubemap</param>
		/// <param name="output">Output cubemap</param>
		/// <param name="inputMip">Input texture's mip as copy source</param>
		/// <param name="outputMip">Output texture's mip as destination</param>
		void copyCubemap_renderThread(RenderCommandList& cmdList, Texture* input, Texture* output, uint32 inputMip = 0, uint32 outputMip = 0);

		/// <summary>
		/// Blit a mip of input cubemap to a mip of output cubemap. Can be different sizes.
		/// </summary>
		/// <param name="cmdList"></param>
		/// <param name="input"></param>
		/// <param name="output"></param>
		/// <param name="inputMip"></param>
		/// <param name="outputMip"></param>
		/// <param name="faceBegin"></param>
		/// <param name="faceEnd"></param>
		void blitCubemap_renderThread(RenderCommandList& cmdList, Texture* input, Texture* output, uint32 inputMip, uint32 outputMip, int32 faceBegin = 0, int32 faceEnd = 5);

		/// <summary>
		/// Render the given equirectangular (panorama) texture to the cubemap texture.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="inputTexture">Input panorama texture2D</param>
		/// <param name="outputTexture">Output textureCube</param>
		/// <param name="outputTextureSize">Output texture size</param>
		/// <param name="faceBegin">First face of output texture to project. Inclusive.</param>
		/// <param name="faceEnd">Last face of output texture to project. Inclusive.</param>
		void projectPanoramaToCubemap_renderThread(RenderCommandList& cmdList, GLuint inputTexture, GLuint outputTexture, uint32 outputTextureSize, int32 faceBegin, int32 faceEnd);

	// Old implementation. Will be removed.
	public:
		/// <summary>
		/// Generate irradiance cubemap from radiance capture cubemap.
		/// NOTE: This is very expensive. Use bakeDiffuseSH_renderThread() if possible.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="inputRadianceCubemap">Input radiance cubemap</param>
		/// <param name="inputDepthCubemap">Input depth cubemap</param>
		/// <param name="bakeDesc">Baking options</param>
		void bakeDiffuseIBL_renderThread(RenderCommandList& cmdList, GLuint inputRadianceCubemap, GLuint inputDepthCubemap, const IrradianceMapBakeDesc& bakeDesc);

		/// <summary>
		/// Render specular IBL cubemap from radiance capture cubemap.
		/// NOTE: This is very expensive. Use bakeReflectionProbe_renderThread() if possible.
		/// </summary>
		/// <param name="cmdList">Render command list</param>
		/// <param name="inputTexture">Input radiance cubemap</param>
		/// <param name="outputTextureSize">Output cubemap size</param>
		/// <param name="numMips">The number of mips to render</param>
		/// <param name="outputTexture">Output cubemap</param>
		void bakeSpecularIBL_renderThread(RenderCommandList& cmdList, GLuint inputTexture, uint32 outputTextureSize, uint32 numMips, GLuint outputTexture);

	private:
		GLuint        dummyVAO;
		GLuint        dummyFBO;          // Dummy FBO for render to a 2D texture or one face of a cubemap
		GLuint        dummyFBO_2color;   // Dummy FBO for two color attachments
		MeshGeometry* fullscreenQuad;
		MeshGeometry* dummyCube;
		matrix4       cubeTransforms[6];
		GLuint        bdfIntegrationMap;

	};

}
