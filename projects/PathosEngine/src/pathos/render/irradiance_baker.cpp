#include "irradiance_baker.h"
#include "pathos/engine.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/render_device.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/engine_util.h"
#include "pathos/util/engine_thread.h"

#include "badger/math/minmax.h"

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

	class BRDFIntegrationMapVS : public ShaderStage {
	public:
		BRDFIntegrationMapVS() : ShaderStage(GL_VERTEX_SHADER, "BRDFIntegrationMapVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class BRDFIntegrationMapFS : public ShaderStage {
	public:
		BRDFIntegrationMapFS() : ShaderStage(GL_FRAGMENT_SHADER, "BRDFIntegrationMapFS") {
			addDefine("FRAGMENT_SHADER", 1);
			addDefine("BRDF_INTEGRATION", 1);
			setFilepath("specular_ibl.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_BRDFIntegrationMap, BRDFIntegrationMapVS, BRDFIntegrationMapFS);

}

namespace pathos {

	GLuint IrradianceBaker::internal_BRDFIntegrationMap = 0xffffffff;
	GLuint IrradianceBaker::dummyVAO = 0;
	GLuint IrradianceBaker::dummyFBO = 0;
	PlaneGeometry* IrradianceBaker::fullscreenQuad = nullptr;
	CubeGeometry* IrradianceBaker::dummyCube = nullptr;
	glm::mat4 IrradianceBaker::cubeTransforms[6];

	GLuint IrradianceBaker::projectToCubemap(GLuint equirectangularMap, uint32 size, const char* debugName) {
		CHECK(isInMainThread());

		GLuint cubemap = 0;
		const std::string debugNameStr = debugName ? debugName : "";
		
		ENQUEUE_RENDER_COMMAND([equirectangularMap, size, cubemapPtr = &cubemap, debugNameStr](RenderCommandList& cmdList) {
			SCOPED_DRAW_EVENT(EquirectangularMapToCubemap);

			GLuint fbo = IrradianceBaker::dummyFBO;
			CubeGeometry* cube = IrradianceBaker::dummyCube;
			
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, cubemapPtr);
			if (debugNameStr.size() > 0) {
				gRenderDevice->objectLabel(GL_TEXTURE, *cubemapPtr, -1, debugNameStr.c_str());
			}

			cmdList.textureParameteri(*cubemapPtr, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*cubemapPtr, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*cubemapPtr, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(*cubemapPtr, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(*cubemapPtr, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			cmdList.textureStorage2D(*cubemapPtr, 1, GL_RGB16F, size, size);

			cmdList.viewport(0, 0, size, size);
			cmdList.disable(GL_DEPTH_TEST);
			cmdList.cullFace(GL_FRONT);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_EquirectangularToCubemap);
			cmdList.useProgram(program.getGLName());
			cmdList.bindTextureUnit(0, equirectangularMap);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

			for (int32 i = 0; i < 6; ++i) {
				const glm::mat4& viewproj = IrradianceBaker::cubeTransforms[i];

				cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, *cubemapPtr, 0, i);
				cmdList.uniformMatrix4fv(0, 1, GL_FALSE, &viewproj[0][0]);

				cube->activate_position(cmdList);
				cube->activateIndexBuffer(cmdList);
				cube->drawPrimitive(cmdList);
			}

			cmdList.enable(GL_DEPTH_TEST);
			cmdList.cullFace(GL_BACK);
		});
		
		FLUSH_RENDER_COMMAND();

		return cubemap;
	}

	void IrradianceBaker::bakeDiffuseIBL_renderThread(
		RenderCommandList& cmdList,
		GLuint inputTexture,
		const IrradianceMapBakeDesc& bakeDesc)
	{
		CHECK(isInRenderThread());
		CHECK(bakeDesc.encoding == EIrradianceMapEncoding::Cubemap || bakeDesc.encoding == EIrradianceMapEncoding::OctahedralNormalVector);
		SCOPED_DRAW_EVENT(BakeDiffuseIBL);

		GLuint fbo = IrradianceBaker::dummyFBO;
		CubeGeometry* cubeGeom = IrradianceBaker::dummyCube;
		constexpr GLint uniform_transform = 0;
		const bool bBakeCubemap = (bakeDesc.encoding == EIrradianceMapEncoding::Cubemap);

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
		cmdList.bindTextureUnit(0, inputTexture);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		if (bBakeCubemap) {
			for (int32 i = 0; i < 6; ++i) {
				const matrix4& viewproj = IrradianceBaker::cubeTransforms[i];

				cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, bakeDesc.renderTarget, 0, i);
				cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &viewproj[0][0]);

				cubeGeom->activate_position(cmdList);
				cubeGeom->activateIndexBuffer(cmdList);
				cubeGeom->drawPrimitive(cmdList);
			}
		} else {
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, bakeDesc.renderTarget, 0);

			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);
		}

		cmdList.enable(GL_DEPTH_TEST);
		if (bBakeCubemap) {
			cmdList.cullFace(GL_BACK);
		}
	}

	GLuint IrradianceBaker::bakeIrradianceMap(
		GLuint inputCubemap,
		uint32 size,
		bool bAutoDestroyCubemap,
		const char* debugName)
	{
		CHECK(isInMainThread());

		GLuint irradianceMap = 0;

		ENQUEUE_RENDER_COMMAND([inputCubemap, size, bAutoDestroyCubemap, irradianceMapPtr = &irradianceMap, debugName](RenderCommandList& cmdList) {
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, irradianceMapPtr);
			if (debugName != nullptr) {
				gRenderDevice->objectLabel(GL_TEXTURE, *irradianceMapPtr, -1, debugName);
			}
			cmdList.textureStorage2D(*irradianceMapPtr, 1, GL_RGB16F, size, size);

			IrradianceMapBakeDesc bakeDesc;
			bakeDesc.encoding = EIrradianceMapEncoding::Cubemap;
			bakeDesc.renderTarget = *irradianceMapPtr;
			bakeDesc.viewportSize = size;
			bakeDiffuseIBL_renderThread(cmdList, inputCubemap, bakeDesc);

			if (bAutoDestroyCubemap) {
				cmdList.deleteTextures(1, &inputCubemap);
			}
		});

		FLUSH_RENDER_COMMAND();

		return irradianceMap;
	}

	void IrradianceBaker::bakeSpecularIBL_renderThread(
		RenderCommandList& cmdList,
		GLuint inputTexture,
		uint32 textureSize,
		uint32 numMips,
		GLuint outputTexture)
	{
		CHECK(isInRenderThread());
		SCOPED_DRAW_EVENT(BakeSpecularIBL);

		GLuint fbo = IrradianceBaker::dummyFBO;
		CubeGeometry* cube = IrradianceBaker::dummyCube;
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
			uint32 mipWidth = (uint32)(textureSize * std::pow(0.5f, mip));
			cmdList.viewport(0, 0, mipWidth, mipWidth);

			float roughness = (float)mip / (float)(numMips - 1);
			cmdList.uniform1f(uniform_roughness, roughness);
			for (uint32 i = 0; i < 6; ++i) {
				const glm::mat4& viewproj = cubeTransforms[i];

				cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, outputTexture, mip, i);
				cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &viewproj[0][0]);

				cube->activate_position(cmdList);
				cube->activateIndexBuffer(cmdList);
				cube->drawPrimitive(cmdList);
			}
		}

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	void IrradianceBaker::bakePrefilteredEnvMap(GLuint cubemap, uint32 size, GLuint& outEnvMap, uint32& outMipLevels, const char* debugName) {
		CHECK(isInMainThread());

		GLuint envMap = 0;
		uint32 maxMipLevels = badger::min(static_cast<uint32>(floor(log2(size)) + 1), 5u);

		ENQUEUE_RENDER_COMMAND([cubemap, size, envMapPtr = &envMap, maxMipLevels, debugName](RenderCommandList& cmdList) {
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, envMapPtr);
			if (debugName != nullptr) {
				gRenderDevice->objectLabel(GL_TEXTURE, *envMapPtr, -1, debugName);
			}
			cmdList.textureStorage2D(*envMapPtr, maxMipLevels, GL_RGB16F, size, size);
			cmdList.generateTextureMipmap(*envMapPtr);

			bakeSpecularIBL_renderThread(cmdList, cubemap, size, maxMipLevels, *envMapPtr);
		});

		FLUSH_RENDER_COMMAND();

		outEnvMap = envMap;
		outMipLevels = maxMipLevels;
	}

	GLuint IrradianceBaker::bakeBRDFIntegrationMap_renderThread(uint32 size, RenderCommandList& cmdList) {
		CHECK(isInRenderThread());

		SCOPED_DRAW_EVENT(BRDFIntegrationMap);

		const GLuint fbo = IrradianceBaker::dummyFBO;
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

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		return brdfLUT;
	}

	void IrradianceBaker::internal_createIrradianceBakerResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		// Dummy VAO
		gRenderDevice->createVertexArrays(1, &IrradianceBaker::dummyVAO);

		// Dummy FBO
		gRenderDevice->createFramebuffers(1, &IrradianceBaker::dummyFBO);
		cmdList.namedFramebufferDrawBuffer(IrradianceBaker::dummyFBO, GL_COLOR_ATTACHMENT0);

		// Dummy meshes
		IrradianceBaker::fullscreenQuad = new PlaneGeometry(2.0f, 2.0f);
		IrradianceBaker::dummyCube = new CubeGeometry(glm::vec3(1.0f));

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		for (int32 i = 0; i < 6; ++i) {
			IrradianceBaker::cubeTransforms[i] = captureProjection * captureViews[i];
		}

		// BRDF integration map
		IrradianceBaker::internal_BRDFIntegrationMap = IrradianceBaker::bakeBRDFIntegrationMap_renderThread(512, cmdList);
		cmdList.objectLabel(GL_TEXTURE, IrradianceBaker::internal_BRDFIntegrationMap, -1, "BRDF integration map");
	}

	void IrradianceBaker::internal_destroyIrradianceBakerResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		gRenderDevice->deleteVertexArrays(1, &IrradianceBaker::dummyVAO);
		gRenderDevice->deleteFramebuffers(1, &IrradianceBaker::dummyFBO);
		gRenderDevice->deleteTextures(1, &IrradianceBaker::internal_BRDFIntegrationMap);

		delete IrradianceBaker::fullscreenQuad;
		delete IrradianceBaker::dummyCube;
	}

	DEFINE_GLOBAL_RENDER_ROUTINE(IrradianceBaker, IrradianceBaker::internal_createIrradianceBakerResources, IrradianceBaker::internal_destroyIrradianceBakerResources);

}
