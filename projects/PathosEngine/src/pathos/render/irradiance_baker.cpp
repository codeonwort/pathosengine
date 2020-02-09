#include "irradiance_baker.h"
#include "render_device.h"
#include "pathos/engine.h"
#include "pathos/shader/shader.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	GLuint IrradianceBaker::equirectangularToCubemap = 0xffffffff;
	GLuint IrradianceBaker::dummyVAO = 0;
	GLuint IrradianceBaker::dummyFBO = 0;
	CubeGeometry* IrradianceBaker::dummyCube = nullptr;

	GLuint IrradianceBaker::bakeCubemap(GLuint equirectangularMap, uint32 size, GLuint oldCubemap) {
		RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();

		SCOPED_DRAW_EVENT(IrradianceBaker);

		GLuint fbo = IrradianceBaker::dummyFBO;
		CubeGeometry* cube = IrradianceBaker::dummyCube;

		GLuint cubemap;
		if(oldCubemap != 0) {
			cubemap = oldCubemap;
		} else {
			cmdList.createTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemap);
			cmdList.textureParameteri(cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(cubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			cmdList.textureStorage2D(cubemap, 1, GL_RGB16F, size, size);
		}

		cmdList.viewport(0, 0, size, size);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		cmdList.useProgram(equirectangularToCubemap);
		cmdList.bindTextureUnit(0, equirectangularMap);

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
			glm::mat4 viewproj = captureProjection * captureViews[i];

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
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

	static void createIrradianceBakerResources(OpenGLDevice* renderDevice) {
		RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

		// Dummy VAO
		cmdList.createVertexArrays(1, &IrradianceBaker::dummyVAO);

		// Dummy FBO
		cmdList.createFramebuffers(1, &IrradianceBaker::dummyFBO);
		cmdList.namedFramebufferDrawBuffer(IrradianceBaker::dummyFBO, GL_COLOR_ATTACHMENT0);

		// Dummy cube
		IrradianceBaker::dummyCube = new CubeGeometry(glm::vec3(1.0f));

		// Equirectangular map to cubemap
		{
			Shader vs(GL_VERTEX_SHADER, "equiToCube_VS");
			Shader fs(GL_FRAGMENT_SHADER, "equiToCube_FS");

			vs.addDefine("VERTEX_SHADER 1");
			fs.addDefine("FRAGMENT_SHADER 1");

			vs.loadSource("equirectangular_to_cube.glsl");
			fs.loadSource("equirectangular_to_cube.glsl");

			IrradianceBaker::equirectangularToCubemap = pathos::createProgram(vs, fs, "equiToCube");
		}

		cmdList.flushAllCommands();
	}

	static void destroyIrradianceBakerResources(OpenGLDevice* renderDevice) {
		RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

		cmdList.deleteVertexArrays(1, &IrradianceBaker::dummyVAO);
		cmdList.deleteFramebuffers(1, &IrradianceBaker::dummyFBO);
		cmdList.deleteProgram(IrradianceBaker::equirectangularToCubemap);

		delete IrradianceBaker::dummyCube;
	}

	// #todo-misc: Define a macro for static initialization
	static struct InitIrradianceBaker_0xDEADBEEF {
		InitIrradianceBaker_0xDEADBEEF() {
			Engine::internal_registerGlobalRenderRoutine(createIrradianceBakerResources, destroyIrradianceBakerResources);
		}
	} _init_irradiance_baker_0xDEADBEEF;

}
