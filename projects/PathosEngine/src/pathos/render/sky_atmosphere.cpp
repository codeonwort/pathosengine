#include "sky_atmosphere.h"
#include "render_device.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"

// #todo-atmosphere: for test
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/engine_util.h"
#include "pathos/util/log.h"

// Precomputed Atmospheric Scattering
namespace pathos {

	class PASFullscreenVS : public ShaderStage {
	public:
		PASFullscreenVS() : ShaderStage(GL_VERTEX_SHADER, "PASFullscreenVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	// Precompute transmittance LUT
	class PASTransmittanceFS : public ShaderStage {
	public:
		PASTransmittanceFS() : ShaderStage(GL_FRAGMENT_SHADER, "PASTransmittanceFS") {
			setFilepath("atmosphere_precompute.glsl");
			addDefine("PRECOMPUTE_TRANSMITTANCE 1");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_PASTransmittance, PASFullscreenVS, PASTransmittanceFS);

	void init_precomputeTransmittance(OpenGLDevice* device) {
		// #todo-atmosphere: for test
		GLuint lut;
		GLuint fbo;
		gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &lut);
		gRenderDevice->objectLabel(GL_TEXTURE, lut, -1, "LUT_Transmittance");
		gRenderDevice->createFramebuffers(1, &fbo);
		gRenderDevice->objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_LUT_Transmittance");

		PlaneGeometry* fullscreenQuad = new PlaneGeometry(1.0, 1.0);

		ENQUEUE_RENDER_COMMAND([lut, fbo, fullscreenQuad](RenderCommandList& cmdList) {
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_PASTransmittance);
			const int32 WIDTH = 64;
			const int32 HEIGHT = 256;

			cmdList.textureStorage2D(lut, 1, GL_RGBA32F, WIDTH, HEIGHT);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
			cmdList.viewport(0, 0, WIDTH, HEIGHT);
			cmdList.useProgram(program.getGLName());
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, lut, 0);
			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);
		});

		LOG(LogDebug, "Generate transmittance LUT (%s)", "LUT_Transmittance");
	}
	void destroy_precomputeTransmittance(OpenGLDevice* device) {
		//
	}

	DEFINE_GLOBAL_RENDER_ROUTINE(PAS, init_precomputeTransmittance, destroy_precomputeTransmittance);
}

namespace pathos {

	struct UBO_Atmosphere {
		glm::vec4 sunParams;     // (sunSizeMultiplier, sunIntensity, ?, ?)
	};

	void AtmosphereScattering::render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera)
	{
		SCOPED_DRAW_EVENT(AtmosphereScattering);

		cmdList.useProgram(program);

		cmdList.depthFunc(GL_GEQUAL);

		UBO_Atmosphere uboData;
		uboData.sunParams.x = 5.0f;
		uboData.sunParams.y = 13.61839144264511f;
		ubo.update(cmdList, 1, &uboData);

		cmdList.bindVertexArray(vao);
		cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
		cmdList.bindVertexArray(0);
	}

	void AtmosphereScattering::onSpawn() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("fullscreen_quad.glsl");
		fs.loadSource("atmosphere.glsl");

		program = pathos::createProgram(vs, fs, "AtmosphereScattering");
		ubo.init<UBO_Atmosphere>();

		gRenderDevice->createVertexArrays(1, &vao);
	}

	void AtmosphereScattering::onDestroy() {
		gRenderDevice->deleteVertexArrays(1, &vao);
		gRenderDevice->deleteProgram(program);
	}

}
