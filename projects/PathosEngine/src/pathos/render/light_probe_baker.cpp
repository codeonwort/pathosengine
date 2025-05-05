#include "light_probe_baker.h"

#include "pathos/render/fullscreen_util.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/texture.h"
#include "pathos/rhi/buffer.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/engine_util.h"
#include "pathos/util/engine_thread.h"
#include "pathos/engine.h"

// Cubemap utils
namespace pathos {
	
	class EquirectangularToCubeVS : public ShaderStage {
	public:
		EquirectangularToCubeVS() : ShaderStage(GL_VERTEX_SHADER, "EquirectangularToCubeVS") {
			addDefine("VERTEX_SHADER", 1);
			setFilepath("equirectangular_to_cube.glsl");
		}
	};
	class EquirectangularToCubeFS : public ShaderStage {
	public:
		EquirectangularToCubeFS() : ShaderStage(GL_FRAGMENT_SHADER, "EquirectangularToCubeFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("equirectangular_to_cube.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_EquirectangularToCubemap, EquirectangularToCubeVS, EquirectangularToCubeFS);

	class CopyCubemapCS : public ShaderStage {
	public:
		CopyCubemapCS() : ShaderStage(GL_COMPUTE_SHADER, "CopyCubemapCS") {
			setFilepath("sky/copy_cubemap.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_CopyCubemap, CopyCubemapCS);

	class BlitCubemapVS : public ShaderStage {
	public:
		BlitCubemapVS() : ShaderStage(GL_VERTEX_SHADER, "BlitCubemapVS") {
			addDefine("VERTEX_SHADER", 1);
			setFilepath("sky/blit_cubemap.glsl");
		}
	};
	class BlitCubemapFS : public ShaderStage {
	public:
		BlitCubemapFS() : ShaderStage(GL_FRAGMENT_SHADER, "BlitCubemapFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("sky/blit_cubemap.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_BlitCubemap, BlitCubemapVS, BlitCubemapFS);

}

// Diffuse GI
namespace pathos {

	template<int CubemapType>
	class DiffuseSHGenCS : public ShaderStage {
	public:
		DiffuseSHGenCS() : ShaderStage(GL_COMPUTE_SHADER, "DiffuseSHGenCS") {
			addDefine("CUBEMAP_TYPE", CubemapType);
			setFilepath("gi/compute_diffuse_sh.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_SkyDiffuseSH, DiffuseSHGenCS<0>);
	DEFINE_COMPUTE_PROGRAM(Program_LightProbeSH, DiffuseSHGenCS<1>);

	class OctahedralDepthAtlasCS : public ShaderStage {
	public:
		OctahedralDepthAtlasCS() : ShaderStage(GL_COMPUTE_SHADER, "OctahedralDepthAtlasCS") {
			setFilepath("gi/octahedral_depth_atlas.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_OctahedralDepthAtlas, OctahedralDepthAtlasCS);

}

// Specular GI
namespace pathos {

	class BRDFIntegrationMapFS : public ShaderStage {
	public:
		BRDFIntegrationMapFS() : ShaderStage(GL_FRAGMENT_SHADER, "BRDFIntegrationMapFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("gi/brdf_integration_map.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_BRDFIntegrationMap, FullscreenVS, BRDFIntegrationMapFS);

	class ReflectionProbeDownsampleCS : public ShaderStage {
	public:
		ReflectionProbeDownsampleCS() : ShaderStage(GL_COMPUTE_SHADER, "ReflectionProbeDownsampleCS") {
			setFilepath("gi/reflection_probe_downsample.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_ReflectionProbeDownsample, ReflectionProbeDownsampleCS);

	class ReflectionProbeFilteringCS : public ShaderStage {
	public:
		ReflectionProbeFilteringCS() : ShaderStage(GL_COMPUTE_SHADER, "ReflectionProbeFilteringCS") {
			setFilepath("gi/reflection_probe_filtering.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_ReflectionProbeFiltering, ReflectionProbeFilteringCS);

}

// Deprecated
namespace pathos {

	template<EIrradianceMapEncoding encoding>
	class IrradianceMapVS : public ShaderStage {
	public:
		IrradianceMapVS() : ShaderStage(GL_VERTEX_SHADER, "IrradianceMapVS") {
			if (encoding == EIrradianceMapEncoding::Cubemap) {
				addDefine("VERTEX_SHADER", 1);
				setFilepath("gi/deprecated_diffuse_irradiance.glsl");
			} else if (encoding == EIrradianceMapEncoding::OctahedralNormalVector) {
				setFilepath("fullscreen_quad.glsl");
			}
		}
	};
	template<EIrradianceMapEncoding encoding>
	class IrradianceMapFS : public ShaderStage {
	public:
		IrradianceMapFS() : ShaderStage(GL_FRAGMENT_SHADER, "IrradianceMapFS") {
			addDefine("FRAGMENT_SHADER", 1);
			if (encoding == EIrradianceMapEncoding::OctahedralNormalVector) {
				addDefine("ONV_ENCODING", 1);
			}
			setFilepath("gi/deprecated_diffuse_irradiance.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_IrradianceMap_Cube, IrradianceMapVS<EIrradianceMapEncoding::Cubemap>, IrradianceMapFS<EIrradianceMapEncoding::Cubemap>);
	DEFINE_SHADER_PROGRAM2(Program_IrradianceMap_ONV, IrradianceMapVS<EIrradianceMapEncoding::OctahedralNormalVector>, IrradianceMapFS<EIrradianceMapEncoding::OctahedralNormalVector>);

	class PrefilterEnvMapVS : public ShaderStage {
	public:
		PrefilterEnvMapVS() : ShaderStage(GL_VERTEX_SHADER, "PrefilterEnvMapVS") {
			addDefine("VERTEX_SHADER", 1);
			addDefine("PREFILTER_ENV_MAP", 1);
			setFilepath("gi/deprecated_specular_ibl.glsl");
		}
	};
	class PrefilterEnvMapFS : public ShaderStage {
	public:
		PrefilterEnvMapFS() : ShaderStage(GL_FRAGMENT_SHADER, "PrefilterEnvMapFS") {
			addDefine("FRAGMENT_SHADER", 1);
			addDefine("PREFILTER_ENV_MAP", 1);
			setFilepath("gi/deprecated_specular_ibl.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_PrefilterEnvMap, PrefilterEnvMapVS, PrefilterEnvMapFS);
}

namespace pathos {

	LightProbeBaker& LightProbeBaker::get() {
		static LightProbeBaker instance;
		return instance;
	}

	LightProbeBaker::LightProbeBaker() {
		bdfIntegrationMap = 0xffffffff;
		dummyVAO = 0;
		dummyFBO = 0;
		dummyFBO_2color = 0;

		matrix4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		matrix4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		for (int32 i = 0; i < 6; ++i) {
			cubeTransforms[i] = captureProjection * captureViews[i];
		}

		fullscreenQuad = nullptr;
		dummyCube = nullptr;
	}

	void LightProbeBaker::initializeResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		// Dummy VAO
		gRenderDevice->createVertexArrays(1, &dummyVAO);

		// Dummy FBO
		gRenderDevice->createFramebuffers(1, &dummyFBO);
		cmdList.namedFramebufferDrawBuffer(dummyFBO, GL_COLOR_ATTACHMENT0);

		static const GLenum drawBuffers_color2[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		gRenderDevice->createFramebuffers(1, &dummyFBO_2color);
		cmdList.namedFramebufferDrawBuffers(dummyFBO_2color, _countof(drawBuffers_color2), drawBuffers_color2);

		// Dummy meshes
		fullscreenQuad = gEngine->getSystemGeometryUnitPlane();
		dummyCube = gEngine->getSystemGeometryUnitCube();

		// BRDF integration map
		bdfIntegrationMap = bakeBRDFIntegrationMap_renderThread(cmdList, 512);
		cmdList.objectLabel(GL_TEXTURE, bdfIntegrationMap, -1, "BRDF integration map");
	}

	void LightProbeBaker::destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		gRenderDevice->deleteVertexArrays(1, &dummyVAO);
		gRenderDevice->deleteFramebuffers(1, &dummyFBO);
		gRenderDevice->deleteFramebuffers(1, &dummyFBO_2color);
		gRenderDevice->deleteTextures(1, &bdfIntegrationMap);
		// Just cleanup references as they are owned by gEngine.
		fullscreenQuad = nullptr;
		dummyCube = nullptr;
	}

	void LightProbeBaker::bakeSkyDiffuseSH_renderThread(RenderCommandList& cmdList, Texture* inCubemap, Buffer* outSH) {
		CHECK(isInRenderThread());
		CHECK(inCubemap->getCreateParams().glDimension == GL_TEXTURE_CUBE_MAP);
		SCOPED_DRAW_EVENT(BakeSkyDiffuseSH);
		SCOPED_GPU_COUNTER(BakeSkyDiffuseSH);
		
		const uint32 cubemapSize = inCubemap->getCreateParams().width;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_SkyDiffuseSH);
		cmdList.useProgram(program.getGLName());

		cmdList.uniform1ui(1, cubemapSize);
		cmdList.uniform1ui(2, 0 /*shIndex*/);

		cmdList.bindTextureUnit(0, inCubemap->internal_getGLName());
		outSH->bindAsSSBO(cmdList, 2);

		cmdList.memoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		cmdList.dispatchCompute(1, 1, 1);

		cmdList.memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		cmdList.bindTextureUnit(0, 0);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	}

	void LightProbeBaker::bakeLightProbeSH_renderThread(RenderCommandList& cmdList, Texture* inColorCubemap, Texture* inDepthCubemap, Buffer* outSH, uint32 shIndex /*= 0*/) {
		CHECK(isInRenderThread());
		CHECK(inColorCubemap->getCreateParams().glDimension == GL_TEXTURE_CUBE_MAP);
		CHECK(inDepthCubemap->getCreateParams().glDimension == GL_TEXTURE_CUBE_MAP);
		CHECK(inColorCubemap->getCreateParams().width == inDepthCubemap->getCreateParams().width);
		SCOPED_DRAW_EVENT(BakeLightProbeSH);
		SCOPED_GPU_COUNTER(BakeLightProbeSH);

		const uint32 cubemapSize = inColorCubemap->getCreateParams().width;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_LightProbeSH);
		cmdList.useProgram(program.getGLName());

		cmdList.uniform1ui(1, cubemapSize);
		cmdList.uniform1ui(2, shIndex);

		cmdList.bindTextureUnit(0, inColorCubemap->internal_getGLName());
		cmdList.bindTextureUnit(1, inDepthCubemap->internal_getGLName());
		outSH->bindAsSSBO(cmdList, 2);

		cmdList.memoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		cmdList.dispatchCompute(1, 1, 1);

		cmdList.memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		cmdList.bindTextureUnit(0, 0);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	}

	void LightProbeBaker::bakeOctahedralDepthAtlas_renderThread(RenderCommandList& cmdList, Texture* inDepthCubemap, GLuint depthAtlasTexture, const vector4ui& atlasTileCoordAndSize) {
		CHECK(isInRenderThread());
		CHECK(inDepthCubemap->getCreateParams().glDimension == GL_TEXTURE_CUBE_MAP);
		SCOPED_DRAW_EVENT(BakeOctahedralDepthAtlas);
		SCOPED_GPU_COUNTER(BakeOctahedralDepthAtlas);

		const uint32 cubemapSize = inDepthCubemap->getCreateParams().width;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_OctahedralDepthAtlas);
		cmdList.useProgram(program.getGLName());

		uint32* uniformMemory = (uint32*)(cmdList.allocateSingleFrameMemory(sizeof(uint32) * 4));
		std::memcpy(uniformMemory, &atlasTileCoordAndSize[0], sizeof(uint32) * 4);
		cmdList.uniform4uiv(1, 1, uniformMemory);

		cmdList.bindTextureUnit(0, inDepthCubemap->internal_getGLName());
		cmdList.bindImageTexture(0, depthAtlasTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);

		cmdList.memoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		uint32 dispatchX = (atlasTileCoordAndSize.z + 7) / 8;
		uint32 dispatchY = (atlasTileCoordAndSize.w + 7) / 8;
		cmdList.dispatchCompute(dispatchX, dispatchY, 1);

		cmdList.memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	GLuint LightProbeBaker::bakeBRDFIntegrationMap_renderThread(RenderCommandList& cmdList, uint32 size) {
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(BRDFIntegrationMap);

		GLuint brdfLUT = 0;

		gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &brdfLUT);
		cmdList.textureParameteri(brdfLUT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(brdfLUT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(brdfLUT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(brdfLUT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureStorage2D(brdfLUT, 1, GL_RG16F, size, size);

		cmdList.viewport(0, 0, size, size);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_BRDFIntegrationMap);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, dummyFBO);
		cmdList.namedFramebufferTexture(dummyFBO, GL_COLOR_ATTACHMENT0, brdfLUT, 0);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		return brdfLUT;
	}

	void LightProbeBaker::bakeReflectionProbe_renderThread(RenderCommandList& cmdList, GLuint srcCubemap, GLuint dstCubemap) {
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(BakeReflectionProbe);
		SCOPED_GPU_COUNTER(BakeReflectionProbe);

		const uint32 BASE_SIZE = 128;
		const uint32 MIP_COUNT = 7;
		const uint32 MIPS_TOTAL_PIXELS = (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4 + 2 * 2 + 1 * 1);

		// Pass 1. Downsample
		{
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_ReflectionProbeDownsample);
			cmdList.useProgram(program.getGLName());

			cmdList.textureParameteri(srcCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			uint32 inputCubemapSize = BASE_SIZE;
			for (uint32 mip = 0; mip < MIP_COUNT; ++mip) {
				const uint32 targetCubemapSize = inputCubemapSize >> 1;
				const uint32 numGroups = (targetCubemapSize + 7) / 8;

				cmdList.textureParameteri(srcCubemap, GL_TEXTURE_BASE_LEVEL, mip);
				cmdList.bindTextureUnit(0, srcCubemap);
				cmdList.bindImageTexture(0, srcCubemap, mip + 1, GL_TRUE, 6, GL_WRITE_ONLY, GL_RGBA16F);

				cmdList.dispatchCompute(numGroups, numGroups, 6);

				inputCubemapSize = targetCubemapSize;
			}
			cmdList.textureParameteri(srcCubemap, GL_TEXTURE_BASE_LEVEL, 0);
		}

		cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// Pass 2. Filtering
		{
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_ReflectionProbeFiltering);
			cmdList.useProgram(program.getGLName());

			cmdList.textureParameteri(dstCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			const uint32 numGroups = (MIPS_TOTAL_PIXELS + 63) / 64;
			cmdList.bindTextureUnit(0, srcCubemap);
			for (uint32 mip = 0; mip < MIP_COUNT; ++mip) {
				cmdList.bindImageTexture(mip, dstCubemap, mip, GL_TRUE, 6, GL_WRITE_ONLY, GL_RGBA16F);
			}

			cmdList.dispatchCompute(numGroups, 6, 1);
		}

		cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	// #todo-rhi: Too slow? 0.3 ms for 128x128 cubemaps?
	void LightProbeBaker::copyCubemap_renderThread(RenderCommandList& cmdList, Texture* input, Texture* output, uint32 inputMip /*= 0*/, uint32 outputMip /*= 0*/) {
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(CopyCubemap);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_CopyCubemap);
		cmdList.useProgram(program.getGLName());

		const auto& inputDesc = input->getCreateParams();
		const auto& outputDesc = output->getCreateParams();
		CHECK(inputDesc.glDimension == GL_TEXTURE_CUBE_MAP && outputDesc.glDimension == GL_TEXTURE_CUBE_MAP);
		CHECK((inputDesc.width >> inputMip) == (outputDesc.width >> outputMip));

		const GLuint mipSize = (inputDesc.width >> inputMip);

		cmdList.uniform1ui(1, mipSize);

		const GLint layer = 0; // Don't care if layere = GL_TRUE
		cmdList.bindImageTexture(0, input->internal_getGLName(), inputMip, GL_TRUE, layer, GL_READ_ONLY, inputDesc.glStorageFormat);
		cmdList.bindImageTexture(1, output->internal_getGLName(), outputMip, GL_TRUE, layer, GL_WRITE_ONLY, outputDesc.glStorageFormat);

		cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		const uint32 groupSize = (mipSize + 7) / 8;
		cmdList.dispatchCompute(groupSize, groupSize, 1);

		cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void LightProbeBaker::blitCubemap_renderThread(RenderCommandList& cmdList, Texture* input, Texture* output, uint32 inputMip, uint32 outputMip, int32 faceBegin, int32 faceEnd) {
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(BlitCubemap);

		const GLuint outputSize = output->getCreateParams().width >> outputMip;

		cmdList.viewport(0, 0, outputSize, outputSize);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_BlitCubemap);
		cmdList.useProgram(program.getGLName());

		cmdList.uniform1f(1, (float)inputMip);
		cmdList.bindTextureUnit(0, input->internal_getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, dummyFBO);

		dummyCube->bindPositionOnlyVAO(cmdList);

		for (int32 i = faceBegin; i <= faceEnd; ++i) {
			const matrix4& viewproj = cubeTransforms[i];

			cmdList.namedFramebufferTextureLayer(dummyFBO, GL_COLOR_ATTACHMENT0, output->internal_getGLName(), outputMip, i);
			cmdList.uniformMatrix4fv(0, 1, GL_FALSE, &viewproj[0][0]);

			dummyCube->drawPrimitive(cmdList);
		}

		dummyCube->unbindVAO(cmdList);

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	void LightProbeBaker::projectPanoramaToCubemap_renderThread(RenderCommandList& cmdList, GLuint inputTexture, GLuint outputTexture, uint32 outputTextureSize, int32 faceBegin, int32 faceEnd) {
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(PanoramaToCubemap);

		cmdList.viewport(0, 0, outputTextureSize, outputTextureSize);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_EquirectangularToCubemap);
		cmdList.useProgram(program.getGLName());
		cmdList.bindTextureUnit(0, inputTexture);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, dummyFBO);

		dummyCube->bindPositionOnlyVAO(cmdList);

		for (int32 i = faceBegin; i <= faceEnd; ++i) {
			const matrix4& viewproj = cubeTransforms[i];

			cmdList.namedFramebufferTextureLayer(dummyFBO, GL_COLOR_ATTACHMENT0, outputTexture, 0, i);
			cmdList.uniformMatrix4fv(0, 1, GL_FALSE, &viewproj[0][0]);

			dummyCube->drawPrimitive(cmdList);
		}

		dummyCube->unbindVAO(cmdList);

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	void LightProbeBaker::deprecated_bakeDiffuseIBL_renderThread(RenderCommandList& cmdList, GLuint inputRadianceCubemap, GLuint inputDepthCubemap, const Deprecated_IrradianceMapBakeDesc& bakeDesc) {
		CHECK_NO_ENTRY();
		CHECK(isInRenderThread());
		CHECK(bakeDesc.encoding == EIrradianceMapEncoding::Cubemap || bakeDesc.encoding == EIrradianceMapEncoding::OctahedralNormalVector);
		SCOPED_DRAW_EVENT(BakeDiffuseIBL);
		SCOPED_GPU_COUNTER(BakeDiffuseIBL);

		const bool bBakeCubemap = (bakeDesc.encoding == EIrradianceMapEncoding::Cubemap);
		GLuint fbo = bBakeCubemap ? dummyFBO : dummyFBO_2color;
		MeshGeometry* cubeGeom = dummyCube;
		constexpr GLint uniform_transform = 0;

		cmdList.textureParameteri(inputRadianceCubemap, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		cmdList.textureParameteri(inputRadianceCubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(inputRadianceCubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(inputRadianceCubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (inputDepthCubemap != 0) {
			cmdList.textureParameteri(inputDepthCubemap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			cmdList.textureParameteri(inputDepthCubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(inputDepthCubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(inputDepthCubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}

		if (bBakeCubemap) {
			cmdList.textureParameteri(bakeDesc.renderTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(bakeDesc.renderTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(bakeDesc.renderTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(bakeDesc.renderTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(bakeDesc.renderTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			cmdList.viewport(0, 0, bakeDesc.viewportSize, bakeDesc.viewportSize);
		} else {
			cmdList.viewport(
				bakeDesc.viewportOffset.x, bakeDesc.viewportOffset.y,
				bakeDesc.viewportSize, bakeDesc.viewportSize);
		}

		cmdList.disable(GL_DEPTH_TEST);
		if (bBakeCubemap) {
			cmdList.cullFace(GL_FRONT);
		}

		GLuint glProgram = 0;
		if (bBakeCubemap) {
			glProgram = FIND_SHADER_PROGRAM(Program_IrradianceMap_Cube).getGLName();
		} else {
			glProgram = FIND_SHADER_PROGRAM(Program_IrradianceMap_ONV).getGLName();
		}
		CHECK(glProgram != 0);

		cmdList.useProgram(glProgram);
		cmdList.bindTextureUnit(0, inputRadianceCubemap);
		cmdList.bindTextureUnit(1, inputDepthCubemap);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		if (bBakeCubemap) {
			for (int32 i = 0; i < 6; ++i) {
				const matrix4& viewproj = cubeTransforms[i];

				cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, bakeDesc.renderTarget, 0, i);
				cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &viewproj[0][0]);

				cubeGeom->bindPositionOnlyVAO(cmdList);
				cubeGeom->drawPrimitive(cmdList);
			}
		} else {
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, bakeDesc.renderTarget, 0);
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT1, bakeDesc.depthTarget, 0);

			fullscreenQuad->bindFullAttributesVAO(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);
		}

		cmdList.enable(GL_DEPTH_TEST);
		if (bBakeCubemap) {
			cmdList.cullFace(GL_BACK);
		}
	}

	void LightProbeBaker::deprecated_bakeSpecularIBL_renderThread(RenderCommandList& cmdList, GLuint inputTexture, uint32 outputTextureSize, uint32 numMips, GLuint outputTexture) {
		CHECK_NO_ENTRY();
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(BakeSpecularIBL);
		SCOPED_GPU_COUNTER(BakeSpecularIBL);

		constexpr GLint uniform_transform = 0;
		constexpr GLint uniform_roughness = 1;

		cmdList.textureParameteri(outputTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(outputTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(outputTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(outputTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		cmdList.textureParameteri(outputTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_PrefilterEnvMap);
		cmdList.useProgram(program.getGLName());
		cmdList.bindTextureUnit(0, inputTexture);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, dummyFBO);

		for (uint32 mip = 0; mip < numMips; ++mip) {
			// resize framebuffer according to mip-level size
			uint32 mipWidth = (uint32)(outputTextureSize * std::pow(0.5f, mip));
			cmdList.viewport(0, 0, mipWidth, mipWidth);

			float roughness = (float)mip / (float)(numMips - 1);
			cmdList.uniform1f(uniform_roughness, roughness);
			for (uint32 i = 0; i < 6; ++i) {
				const matrix4& viewproj = cubeTransforms[i];

				cmdList.namedFramebufferTextureLayer(dummyFBO, GL_COLOR_ATTACHMENT0, outputTexture, mip, i);
				cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &viewproj[0][0]);

				dummyCube->bindPositionOnlyVAO(cmdList);
				dummyCube->drawPrimitive(cmdList);
			}
		}

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	void LightProbeBaker::static_initializeResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		LightProbeBaker::get().initializeResources(renderDevice, cmdList);
	}

	void LightProbeBaker::static_destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		LightProbeBaker::get().destroyResources(renderDevice, cmdList);
	}

	DEFINE_GLOBAL_RENDER_ROUTINE(LightProbeBaker, LightProbeBaker::static_initializeResources, LightProbeBaker::static_destroyResources);

}
