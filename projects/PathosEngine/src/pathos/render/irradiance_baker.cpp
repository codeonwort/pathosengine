#include "irradiance_baker.h"
#include "render_device.h"
#include "pathos/engine.h"
#include "pathos/shader/shader.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/math_lib.h"

namespace pathos {

	GLuint IrradianceBaker::equirectangularToCubemap = 0xffffffff;
	GLuint IrradianceBaker::diffuseIrradianceShader = 0xffffffff;
	GLuint IrradianceBaker::prefilterEnvMapShader = 0xffffffff;
	GLuint IrradianceBaker::BRDFIntegrationMapShader = 0xffffffff;
	GLuint IrradianceBaker::internal_BRDFIntegrationMap = 0xffffffff;
	GLuint IrradianceBaker::dummyVAO = 0;
	GLuint IrradianceBaker::dummyFBO = 0;
	PlaneGeometry* IrradianceBaker::fullscreenQuad = nullptr;
	CubeGeometry* IrradianceBaker::dummyCube = nullptr;
	glm::mat4 IrradianceBaker::cubeTransforms[6];

	GLuint IrradianceBaker::bakeCubemap(GLuint equirectangularMap, uint32 size) {
		RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
		SCOPED_DRAW_EVENT(EquirectangularMapToCubemap);

		GLuint fbo = IrradianceBaker::dummyFBO;
		CubeGeometry* cube = IrradianceBaker::dummyCube;

		GLuint cubemap;
		cmdList.createTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemap);
		cmdList.textureParameteri(cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(cubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureStorage2D(cubemap, 1, GL_RGB16F, size, size);

		cmdList.viewport(0, 0, size, size);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		cmdList.useProgram(equirectangularToCubemap);
		cmdList.bindTextureUnit(0, equirectangularMap);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		for (int32 i = 0; i < 6; ++i) {
			const glm::mat4& viewproj = cubeTransforms[i];

			cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, cubemap, 0, i);
			cmdList.uniformMatrix4fv(0, 1, GL_FALSE, &viewproj[0][0]);

			cube->activate_position(cmdList);
			cube->activateIndexBuffer(cmdList);
			cube->drawPrimitive(cmdList);
		}

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);

		cmdList.flushAllCommands();

		return cubemap;
	}

	GLuint IrradianceBaker::bakeIrradianceMap(GLuint cubemap, uint32 size, bool autoDestroyCubemap) {
		RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
		SCOPED_DRAW_EVENT(IrradianceMapFromCubemap);

		GLuint fbo = IrradianceBaker::dummyFBO;
		CubeGeometry* cube = IrradianceBaker::dummyCube;

		GLuint irradianceMap;
		cmdList.createTextures(GL_TEXTURE_CUBE_MAP, 1, &irradianceMap);
		cmdList.textureParameteri(irradianceMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(irradianceMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(irradianceMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(irradianceMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(irradianceMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureStorage2D(irradianceMap, 1, GL_RGB16F, size, size);

		cmdList.viewport(0, 0, size, size);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		cmdList.useProgram(diffuseIrradianceShader);
		cmdList.bindTextureUnit(0, cubemap);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		for (int32 i = 0; i < 6; ++i) {
			const glm::mat4& viewproj = cubeTransforms[i];

			cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, irradianceMap, 0, i);
			cmdList.uniformMatrix4fv(0, 1, GL_FALSE, &viewproj[0][0]);

			cube->activate_position(cmdList);
			cube->activateIndexBuffer(cmdList);
			cube->drawPrimitive(cmdList);
		}

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);

		cmdList.flushAllCommands();

		if(autoDestroyCubemap) {
			cmdList.deleteTextures(1, &cubemap);
		}

		return irradianceMap;
	}

	void IrradianceBaker::bakePrefilteredEnvMap(GLuint cubemap, uint32 size, GLuint& outEnvMap, uint32& outMipLevels) {
		RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
		SCOPED_DRAW_EVENT(PrefilteredEnvMap);

		GLuint fbo = IrradianceBaker::dummyFBO;
		CubeGeometry* cube = IrradianceBaker::dummyCube;
		constexpr GLint uniform_transform = 0;
		constexpr GLint uniform_roughness = 1;

		uint32 maxMipLevels = pathos::min(static_cast<uint32>(floor(log2(size)) + 1), 5u);

		GLuint envMap;
		cmdList.createTextures(GL_TEXTURE_CUBE_MAP, 1, &envMap);
		cmdList.textureParameteri(envMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(envMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(envMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(envMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		cmdList.textureParameteri(envMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureStorage2D(envMap, maxMipLevels, GL_RGB16F, size, size);
		cmdList.generateTextureMipmap(envMap);

		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		cmdList.useProgram(prefilterEnvMapShader);
		cmdList.bindTextureUnit(0, cubemap);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		for (uint32 mip = 0; mip < maxMipLevels; ++mip) {
			// resize framebuffer according to mip-level size
			uint32 mipWidth = (uint32)(size * std::pow(0.5f, mip));
			uint32 mipHeight = (uint32)(size * std::pow(0.5f, mip));
			cmdList.viewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			cmdList.uniform1f(uniform_roughness, roughness);
			for (uint32 i = 0; i < 6; ++i) {
				const glm::mat4& viewproj = cubeTransforms[i];

				cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, envMap, mip, i);
				cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &viewproj[0][0]);

				cube->activate_position(cmdList);
				cube->activateIndexBuffer(cmdList);
				cube->drawPrimitive(cmdList);
			}
		}

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);

		cmdList.flushAllCommands();

		outEnvMap = envMap;
		outMipLevels = maxMipLevels;
	}

	GLuint IrradianceBaker::bakeBRDFIntegrationMap(uint32 size) {
		RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
		SCOPED_DRAW_EVENT(BRDFIntegrationMap);

		const GLuint fbo = IrradianceBaker::dummyFBO;

		GLuint brdfLUT;
		cmdList.createTextures(GL_TEXTURE_2D, 1, &brdfLUT);
		cmdList.textureParameteri(brdfLUT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(brdfLUT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(brdfLUT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(brdfLUT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureStorage2D(brdfLUT, 1, GL_RG16F, size, size);

		cmdList.viewport(0, 0, size, size);
		cmdList.useProgram(BRDFIntegrationMapShader);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, brdfLUT, 0);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		cmdList.flushAllCommands();

		return brdfLUT;
	}

	void IrradianceBaker::internal_createIrradianceBakerResources(OpenGLDevice* renderDevice) {
		RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

		// Dummy VAO
		cmdList.createVertexArrays(1, &IrradianceBaker::dummyVAO);

		// Dummy FBO
		cmdList.createFramebuffers(1, &IrradianceBaker::dummyFBO);
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

		// Equirectangular map to cubemap
		{
			Shader vs(GL_VERTEX_SHADER, "equiToCube_VS");
			Shader fs(GL_FRAGMENT_SHADER, "equiToCube_FS");

			vs.addDefine("VERTEX_SHADER 1");
			fs.addDefine("FRAGMENT_SHADER 1");

			vs.loadSource("equirectangular_to_cube.glsl");
			fs.loadSource("equirectangular_to_cube.glsl");

			IrradianceBaker::equirectangularToCubemap = pathos::createProgram(vs, fs, "EquirectangularToCubemap");
		}

		// Irradiance map generator
		{
			Shader vs(GL_VERTEX_SHADER, "irradianceMap_VS");
			Shader fs(GL_FRAGMENT_SHADER, "irradianceMap_FS");

			vs.addDefine("VERTEX_SHADER 1");
			fs.addDefine("FRAGMENT_SHADER 1");

			vs.loadSource("diffuse_irradiance.glsl");
			fs.loadSource("diffuse_irradiance.glsl");

			IrradianceBaker::diffuseIrradianceShader = pathos::createProgram(vs, fs, "DiffuseIrradiance");
		}

		// Pre-filter environment map
		{
			Shader vs(GL_VERTEX_SHADER, "prefilterEnvMap_VS");
			Shader fs(GL_FRAGMENT_SHADER, "prefilterEnvMap_FS");

			vs.addDefine("VERTEX_SHADER 1");
			vs.addDefine("PREFILTER_ENV_MAP 1");
			fs.addDefine("FRAGMENT_SHADER 1");
			fs.addDefine("PREFILTER_ENV_MAP 1");

			vs.loadSource("specular_ibl.glsl");
			fs.loadSource("specular_ibl.glsl");

			IrradianceBaker::prefilterEnvMapShader = pathos::createProgram(vs, fs, "PrefilterEnvMap");
		}

		// BRDF integration map
		{
			Shader vs(GL_VERTEX_SHADER, "BRDFIntegrationMap_VS");
			Shader fs(GL_FRAGMENT_SHADER, "BRDFIntegrationMap_FS");

			fs.addDefine("FRAGMENT_SHADER 1");
			fs.addDefine("BRDF_INTEGRATION 1");

			vs.loadSource("fullscreen_quad.glsl");
			fs.loadSource("specular_ibl.glsl");

			IrradianceBaker::BRDFIntegrationMapShader = pathos::createProgram(vs, fs, "BRDFIntegrationMap");

			IrradianceBaker::internal_BRDFIntegrationMap = IrradianceBaker::bakeBRDFIntegrationMap(512);
			cmdList.objectLabel(GL_TEXTURE, IrradianceBaker::internal_BRDFIntegrationMap, -1, "BRDF integration map");
		}

		cmdList.flushAllCommands();
	}

	void IrradianceBaker::internal_destroyIrradianceBakerResources(OpenGLDevice* renderDevice) {
		RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

		cmdList.deleteVertexArrays(1, &IrradianceBaker::dummyVAO);
		cmdList.deleteFramebuffers(1, &IrradianceBaker::dummyFBO);
		cmdList.deleteProgram(IrradianceBaker::equirectangularToCubemap);
		cmdList.deleteProgram(IrradianceBaker::diffuseIrradianceShader);
		cmdList.deleteProgram(IrradianceBaker::prefilterEnvMapShader);
		cmdList.deleteProgram(IrradianceBaker::BRDFIntegrationMapShader);
		cmdList.deleteTextures(1, &IrradianceBaker::internal_BRDFIntegrationMap);

		delete IrradianceBaker::fullscreenQuad;
		delete IrradianceBaker::dummyCube;
	}

	// #todo-misc: Define a macro for static initialization
	static struct InitIrradianceBaker_0xDEADBEEF {
		InitIrradianceBaker_0xDEADBEEF() {
			Engine::internal_registerGlobalRenderRoutine(IrradianceBaker::internal_createIrradianceBakerResources, IrradianceBaker::internal_destroyIrradianceBakerResources);
		}
	} _init_irradiance_baker_0xDEADBEEF;

}
