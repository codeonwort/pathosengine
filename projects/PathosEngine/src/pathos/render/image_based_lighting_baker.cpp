#include "image_based_lighting_baker.h"

#include "pathos/render/fullscreen_util.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/render_device.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/engine_util.h"
#include "pathos/util/engine_thread.h"
#include "pathos/engine.h"

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

	template<EIrradianceMapEncoding encoding>
	class IrradianceMapVS : public ShaderStage {
	public:
		IrradianceMapVS() : ShaderStage(GL_VERTEX_SHADER, "IrradianceMapVS") {
			if (encoding == EIrradianceMapEncoding::Cubemap) {
				addDefine("VERTEX_SHADER", 1);
				setFilepath("diffuse_irradiance.glsl");
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
			setFilepath("diffuse_irradiance.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_IrradianceMap_Cube, IrradianceMapVS<EIrradianceMapEncoding::Cubemap>, IrradianceMapFS<EIrradianceMapEncoding::Cubemap>);
	DEFINE_SHADER_PROGRAM2(Program_IrradianceMap_ONV, IrradianceMapVS<EIrradianceMapEncoding::OctahedralNormalVector>, IrradianceMapFS<EIrradianceMapEncoding::OctahedralNormalVector>);

	class PrefilterEnvMapVS : public ShaderStage {
	public:
		PrefilterEnvMapVS() : ShaderStage(GL_VERTEX_SHADER, "PrefilterEnvMapVS") {
			addDefine("VERTEX_SHADER", 1);
			addDefine("PREFILTER_ENV_MAP", 1);
			setFilepath("specular_ibl.glsl");
		}
	};
	class PrefilterEnvMapFS : public ShaderStage {
	public:
		PrefilterEnvMapFS() : ShaderStage(GL_FRAGMENT_SHADER, "PrefilterEnvMapFS") {
			addDefine("FRAGMENT_SHADER", 1);
			addDefine("PREFILTER_ENV_MAP", 1);
			setFilepath("specular_ibl.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_PrefilterEnvMap, PrefilterEnvMapVS, PrefilterEnvMapFS);

	class BRDFIntegrationMapFS : public ShaderStage {
	public:
		BRDFIntegrationMapFS() : ShaderStage(GL_FRAGMENT_SHADER, "BRDFIntegrationMapFS") {
			addDefine("FRAGMENT_SHADER", 1);
			addDefine("BRDF_INTEGRATION", 1);
			setFilepath("specular_ibl.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_BRDFIntegrationMap, FullscreenVS, BRDFIntegrationMapFS);

}

namespace pathos {

	class ReflectionProbeDownsampleCS : public ShaderStage {
	public:
		ReflectionProbeDownsampleCS() : ShaderStage(GL_COMPUTE_SHADER, "ReflectionProbeDownsampleCS") {
			setFilepath("reflection_probe_downsample.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_ReflectionProbeDownsample, ReflectionProbeDownsampleCS);

	class ReflectionProbeFilteringCS : public ShaderStage {
	public:
		ReflectionProbeFilteringCS() : ShaderStage(GL_COMPUTE_SHADER, "ReflectionProbeFilteringCS") {
			setFilepath("reflection_probe_filtering.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_ReflectionProbeFiltering, ReflectionProbeFilteringCS);

}

namespace pathos {

	GLuint ImageBasedLightingBaker::internal_BRDFIntegrationMap = 0xffffffff;
	GLuint ImageBasedLightingBaker::dummyVAO = 0;
	GLuint ImageBasedLightingBaker::dummyFBO = 0;
	GLuint ImageBasedLightingBaker::dummyFBO_2color = 0;
	MeshGeometry* ImageBasedLightingBaker::fullscreenQuad = nullptr;
	MeshGeometry* ImageBasedLightingBaker::dummyCube = nullptr;
	matrix4 ImageBasedLightingBaker::cubeTransforms[6];

	void ImageBasedLightingBaker::projectPanoramaToCubemap_renderThread(
		RenderCommandList& cmdList,
		GLuint inputTexture,
		GLuint outputTexture,
		uint32 outputTextureSize)
	{
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(PanoramaToCubemap);

		GLuint fbo = ImageBasedLightingBaker::dummyFBO;
		MeshGeometry* cube = ImageBasedLightingBaker::dummyCube;

		cmdList.viewport(0, 0, outputTextureSize, outputTextureSize);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_EquirectangularToCubemap);
		cmdList.useProgram(program.getGLName());
		cmdList.bindTextureUnit(0, inputTexture);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		cube->bindPositionOnlyVAO(cmdList);

		for (int32 i = 0; i < 6; ++i) {
			const matrix4& viewproj = ImageBasedLightingBaker::cubeTransforms[i];

			cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, outputTexture, 0, i);
			cmdList.uniformMatrix4fv(0, 1, GL_FALSE, &viewproj[0][0]);

			cube->drawPrimitive(cmdList);
		}

		cube->unbindVAO(cmdList);

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	void ImageBasedLightingBaker::bakeDiffuseIBL_renderThread(
		RenderCommandList& cmdList,
		GLuint inputRadianceCubemap,
		GLuint inputDepthCubemap,
		const IrradianceMapBakeDesc& bakeDesc)
	{
		CHECK(isInRenderThread());
		CHECK(bakeDesc.encoding == EIrradianceMapEncoding::Cubemap || bakeDesc.encoding == EIrradianceMapEncoding::OctahedralNormalVector);
		SCOPED_DRAW_EVENT(BakeDiffuseIBL);

		const bool bBakeCubemap = (bakeDesc.encoding == EIrradianceMapEncoding::Cubemap);
		GLuint fbo = bBakeCubemap ? ImageBasedLightingBaker::dummyFBO : ImageBasedLightingBaker::dummyFBO_2color;
		MeshGeometry* cubeGeom = ImageBasedLightingBaker::dummyCube;
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
				const matrix4& viewproj = ImageBasedLightingBaker::cubeTransforms[i];

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

	void ImageBasedLightingBaker::bakeSkyIrradianceMap_renderThread(
		RenderCommandList& cmdList,
		GLuint inputSkyCubemap,
		GLuint targetCubemap,
		uint32 targetSize)
	{
		CHECK(isInRenderThread());
		
		IrradianceMapBakeDesc bakeDesc;
		bakeDesc.encoding     = EIrradianceMapEncoding::Cubemap;
		bakeDesc.renderTarget = targetCubemap;
		bakeDesc.depthTarget  = 0;
		bakeDesc.viewportSize = targetSize;
		bakeDiffuseIBL_renderThread(cmdList, inputSkyCubemap, 0, bakeDesc);
	}

	void ImageBasedLightingBaker::bakeReflectionProbe_renderThread(RenderCommandList& cmdList, GLuint srcCubemap, GLuint dstCubemap) {
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(BakeReflectionProbe);

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
	}

	void ImageBasedLightingBaker::bakeSpecularIBL_renderThread(
		RenderCommandList& cmdList,
		GLuint inputTexture,
		uint32 outputTextureSize,
		uint32 numMips,
		GLuint outputTexture)
	{
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(BakeSpecularIBL);

		GLuint fbo = ImageBasedLightingBaker::dummyFBO;
		MeshGeometry* cube = ImageBasedLightingBaker::dummyCube;
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

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		for (uint32 mip = 0; mip < numMips; ++mip) {
			// resize framebuffer according to mip-level size
			uint32 mipWidth = (uint32)(outputTextureSize * std::pow(0.5f, mip));
			cmdList.viewport(0, 0, mipWidth, mipWidth);

			float roughness = (float)mip / (float)(numMips - 1);
			cmdList.uniform1f(uniform_roughness, roughness);
			for (uint32 i = 0; i < 6; ++i) {
				const matrix4& viewproj = cubeTransforms[i];

				cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, outputTexture, mip, i);
				cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &viewproj[0][0]);

				cube->bindPositionOnlyVAO(cmdList);
				cube->drawPrimitive(cmdList);
			}
		}

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	GLuint ImageBasedLightingBaker::bakeBRDFIntegrationMap_renderThread(uint32 size, RenderCommandList& cmdList) {
		CHECK(isInRenderThread());

		SCOPED_DRAW_EVENT(BRDFIntegrationMap);

		const GLuint fbo = ImageBasedLightingBaker::dummyFBO;
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

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, brdfLUT, 0);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		return brdfLUT;
	}

	void ImageBasedLightingBaker::internal_createIrradianceBakerResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		// Dummy VAO
		gRenderDevice->createVertexArrays(1, &ImageBasedLightingBaker::dummyVAO);

		// Dummy FBO
		gRenderDevice->createFramebuffers(1, &ImageBasedLightingBaker::dummyFBO);
		cmdList.namedFramebufferDrawBuffer(ImageBasedLightingBaker::dummyFBO, GL_COLOR_ATTACHMENT0);

		static const GLenum drawBuffers_color2[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		gRenderDevice->createFramebuffers(1, &ImageBasedLightingBaker::dummyFBO_2color);
		cmdList.namedFramebufferDrawBuffers(ImageBasedLightingBaker::dummyFBO_2color, _countof(drawBuffers_color2), drawBuffers_color2);

		// Dummy meshes
		ImageBasedLightingBaker::fullscreenQuad = gEngine->getSystemGeometryUnitPlane();
		ImageBasedLightingBaker::dummyCube = gEngine->getSystemGeometryUnitCube();

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
			ImageBasedLightingBaker::cubeTransforms[i] = captureProjection * captureViews[i];
		}

		// BRDF integration map
		ImageBasedLightingBaker::internal_BRDFIntegrationMap = ImageBasedLightingBaker::bakeBRDFIntegrationMap_renderThread(512, cmdList);
		cmdList.objectLabel(GL_TEXTURE, ImageBasedLightingBaker::internal_BRDFIntegrationMap, -1, "BRDF integration map");
	}

	void ImageBasedLightingBaker::internal_destroyIrradianceBakerResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		gRenderDevice->deleteVertexArrays(1, &ImageBasedLightingBaker::dummyVAO);
		gRenderDevice->deleteFramebuffers(1, &ImageBasedLightingBaker::dummyFBO);
		gRenderDevice->deleteFramebuffers(1, &ImageBasedLightingBaker::dummyFBO_2color);
		gRenderDevice->deleteTextures(1, &ImageBasedLightingBaker::internal_BRDFIntegrationMap);
		// Just cleanup references as they are owned by gEngine.
		ImageBasedLightingBaker::fullscreenQuad = nullptr;
		ImageBasedLightingBaker::dummyCube = nullptr;
	}

	DEFINE_GLOBAL_RENDER_ROUTINE(ImageBasedLightingBaker, ImageBasedLightingBaker::internal_createIrradianceBakerResources, ImageBasedLightingBaker::internal_destroyIrradianceBakerResources);

}
