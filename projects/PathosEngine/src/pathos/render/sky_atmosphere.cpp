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

	// #todo-atmosphere: better lifecycle management
	GLuint gTransmittanceLUT = 0;

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

	class AtmosphericScatteringFS : public ShaderStage {
	public:
		AtmosphericScatteringFS() : ShaderStage(GL_FRAGMENT_SHADER, "AtmosphericScatteringFS") {
			setFilepath("atmosphere.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_PASTransmittance, PASFullscreenVS, PASTransmittanceFS);
	DEFINE_SHADER_PROGRAM2(Program_AtmosphericScattering, PASFullscreenVS, AtmosphericScatteringFS);

	void init_precomputeTransmittance(OpenGLDevice* device) {
		GLuint& lut = gTransmittanceLUT;
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
			// This does matter; transmittance is 1 outside of atmosphere.
			//GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

			cmdList.textureStorage2D(lut, 1, GL_RGBA32F, WIDTH, HEIGHT);
			cmdList.textureParameteri(lut, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			cmdList.textureParameteri(lut, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			cmdList.textureParameterfv(lut, GL_TEXTURE_BORDER_COLOR, borderColor);

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
		gRenderDevice->deleteTextures(1, &gTransmittanceLUT);
		gTransmittanceLUT = 0;
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

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AtmosphericScattering);
		cmdList.useProgram(program.getGLName());

		cmdList.depthFunc(GL_GEQUAL);

		UBO_Atmosphere uboData;
		uboData.sunParams.x = 5.0f;
		uboData.sunParams.y = 13.61839144264511f;
		ubo.update(cmdList, 1, &uboData);

		cmdList.bindTextureUnit(0, gTransmittanceLUT);
		cmdList.bindVertexArray(vao);
		cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
		cmdList.bindVertexArray(0);
	}

	void AtmosphereScattering::onSpawn() {
		ubo.init<UBO_Atmosphere>();
		gRenderDevice->createVertexArrays(1, &vao);
	}

	void AtmosphereScattering::onDestroy() {
		gRenderDevice->deleteVertexArrays(1, &vao);
	}

}