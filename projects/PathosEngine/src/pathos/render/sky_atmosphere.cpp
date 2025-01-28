#include "sky_atmosphere.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/image_based_lighting_baker.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/scene/sky_atmosphere_component.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/engine_util.h"
#include "pathos/util/log.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

// Precomputed Atmospheric Scattering
namespace pathos {

	static ConsoleVariable<float> cvar_sunDisk("r.atmosphere.sunDiskSize", 5.0f, "Sun disk size");

	static constexpr uint32 LUT_WIDTH = 64;
	static constexpr uint32 LUT_HEIGHT = 256;
	static constexpr uint32 TO_CUBEMAP_SIZE = pathos::SKY_PREFILTER_MAP_DEFAULT_SIZE;

	struct UBO_Atmosphere {
		static constexpr uint32 BINDING_POINT = 1;

		vector3 sunIntensity;
		float   sunDiskSize;

		vector2 screenFlip; // (flipX, flipY)
		uint32  renderToCubemap;
		float   cubemapSize;

		matrix4 customViewInv;
	};

	template<bool bCheckReverseZ>
	class PASFullscreenVS : public ShaderStage {
	public:
		PASFullscreenVS() : ShaderStage(GL_VERTEX_SHADER, "PASFullscreenVS") {
			if (bCheckReverseZ && pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("FORCE_Z_TO_ZERO", 1);
			}
			setFilepath("fullscreen_quad.glsl");
		}
	};

	// Precompute transmittance LUT
	class PASTransmittanceFS : public ShaderStage {
	public:
		PASTransmittanceFS() : ShaderStage(GL_FRAGMENT_SHADER, "PASTransmittanceFS") {
			addDefine("PRECOMPUTE_TRANSMITTANCE", 1);
			setFilepath("sky/atmosphere_precompute.glsl");
		}
	};

	class AtmosphericScatteringFS : public ShaderStage {
	public:
		AtmosphericScatteringFS() : ShaderStage(GL_FRAGMENT_SHADER, "AtmosphericScatteringFS") {
			setFilepath("sky/atmosphere.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_PASTransmittance, PASFullscreenVS<false>, PASTransmittanceFS);
	DEFINE_SHADER_PROGRAM2(Program_AtmosphericScattering, PASFullscreenVS<true>, AtmosphericScatteringFS);

}

namespace pathos {

	void SkyAtmospherePass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_SkyAtmosphere");
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemapTexture);
		cmdList.textureStorage2D(cubemapTexture, 1, GL_RGBA16F, TO_CUBEMAP_SIZE, TO_CUBEMAP_SIZE);

		ubo.init<UBO_Atmosphere>("UBO_SkyAtmosphere");
		gRenderDevice->createVertexArrays(1, &vao);
	}

	void SkyAtmospherePass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		gRenderDevice->deleteTextures(1, &cubemapTexture);
		gRenderDevice->deleteVertexArrays(1, &vao);
		if (transmittanceLUT) delete transmittanceLUT;
	}

	void SkyAtmospherePass::renderSkyAtmosphere(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera, MeshGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(SkyAtmosphere);

		renderTransmittanceLUT(cmdList, fullscreenQuad);
		renderToScreen(cmdList, scene, camera);
		if (scene->skyAtmosphere->bLightingDirty) {
			renderToCubemap(cmdList, scene);
			renderSkyIrradianceMap(cmdList, scene);
			renderSkyPrefilterMap(cmdList, scene);
		}
	}

	void SkyAtmospherePass::renderToScreen(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) {
		SCOPED_DRAW_EVENT(SkyAtmosphereToScreen);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		vector3 sunIntensity = vector3(13.61839144264511f);
		if (scene->proxyList_directionalLight.size() > 0) {
			sunIntensity = scene->proxyList_directionalLight[0]->getIntensity();
		}

		UBO_Atmosphere uboData;
		uboData.sunIntensity    = sunIntensity;
		uboData.sunDiskSize     = cvar_sunDisk.getFloat();
		uboData.screenFlip.x    = camera->getLens().isFlipX() ? -1.0f : 1.0f;
		uboData.screenFlip.y    = camera->getLens().isFlipY() ? -1.0f : 1.0f;
		uboData.renderToCubemap = 0;
		ubo.update(cmdList, UBO_Atmosphere::BINDING_POINT, &uboData);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AtmosphericScattering);
		cmdList.useProgram(program.getGLName());

		// Write to only far plane.
		cmdList.depthFunc(GL_EQUAL);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthMask(GL_FALSE);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);
		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		cmdList.bindTextureUnit(0, transmittanceLUT->internal_getGLName());
		cmdList.bindVertexArray(vao);
		cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);

		cmdList.bindVertexArray(0);
	}

	void SkyAtmospherePass::renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyAtmosphereToCubemap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		const vector3 lookAtOffsets[6] = {
			vector3(+1.0f, 0.0f, 0.0f), // posX
			vector3(-1.0f, 0.0f, 0.0f), // negX
			vector3(0.0f, +1.0f, 0.0f), // posY
			vector3(0.0f, -1.0f, 0.0f), // negY
			vector3(0.0f, 0.0f, +1.0f), // posZ
			vector3(0.0f, 0.0f, -1.0f), // negZ
		};
		const vector3 upVectors[6] = {
			vector3(0.0f, -1.0f, 0.0f), // posX
			vector3(0.0f, -1.0f, 0.0f), // negX
			vector3(+1.0f, 0.0f, 0.0f), // posY
			vector3(-1.0f, 0.0f, 0.0f), // negY
			vector3(0.0f, -1.0f, 0.0f), // posZ
			vector3(0.0f, -1.0f, 0.0f), // negZ
		};

		Camera tempCamera(PerspectiveLens(90.0f, 1.0f, 0.1f, 1000.0f));

		vector3 sunIntensity = vector3(13.61839144264511f);
		if (scene->proxyList_directionalLight.size() > 0) {
			sunIntensity = scene->proxyList_directionalLight[0]->getIntensity();
		}
		float sunDiskSize = cvar_sunDisk.getFloat();

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AtmosphericScattering);
		cmdList.useProgram(program.getGLName());

		cmdList.disable(GL_DEPTH_TEST);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.viewport(0, 0, TO_CUBEMAP_SIZE, TO_CUBEMAP_SIZE);

		cmdList.bindTextureUnit(0, transmittanceLUT->internal_getGLName());
		cmdList.bindVertexArray(vao);

		for (int32 i = 0; i < 6; ++i) {
			tempCamera.lookAt(vector3(0.0f), lookAtOffsets[i], upVectors[i]);
			bool flipScreenXY = (i != 2 && i != 3);

			UBO_Atmosphere uboData;
			uboData.sunIntensity    = sunIntensity;
			uboData.sunDiskSize     = sunDiskSize;
			uboData.screenFlip.x    = flipScreenXY ? -1.0f : 1.0f;
			uboData.screenFlip.y    = flipScreenXY ? -1.0f : 1.0f;
			uboData.renderToCubemap = 1;
			uboData.cubemapSize     = (float)TO_CUBEMAP_SIZE;
			uboData.customViewInv   = glm::inverse(tempCamera.getViewMatrix());
			ubo.update(cmdList, UBO_Atmosphere::BINDING_POINT, &uboData);

			cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, cubemapTexture, 0, i);
			cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		cmdList.bindVertexArray(0);
	}

	void SkyAtmospherePass::renderSkyIrradianceMap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyAtmosphereToIrradianceMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ImageBasedLightingBaker::bakeSkyIrradianceMap_renderThread(
			cmdList,
			cubemapTexture,
			sceneContext.skyIrradianceMap,
			pathos::SKY_IRRADIANCE_MAP_SIZE);
	}

	void SkyAtmospherePass::renderSkyPrefilterMap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyAtmosphereToPrefilterMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		sceneContext.reallocSkyPrefilterMap(cmdList, TO_CUBEMAP_SIZE);

		ImageBasedLightingBaker::bakeSpecularIBL_renderThread(
			cmdList,
			cubemapTexture,
			TO_CUBEMAP_SIZE,
			sceneContext.skyPrefilterMapMipCount,
			sceneContext.skyPrefilteredMap);
	}

	void SkyAtmospherePass::renderTransmittanceLUT(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad) {
		if (transmittanceLUT != nullptr) {
			return;
		}

		// Create LUT texture

		TextureCreateParams createParams;
		createParams.width           = LUT_WIDTH;
		createParams.height          = LUT_HEIGHT;
		createParams.depth           = 1;
		createParams.mipLevels       = 1;
		createParams.glDimension     = GL_TEXTURE_2D;
		createParams.glStorageFormat = GL_RGBA32F;
		createParams.debugName       = "Texture_TransmittanceLUT";

		transmittanceLUT = new Texture(createParams);
		transmittanceLUT->createGPUResource();

		const GLuint lut = transmittanceLUT->internal_getGLName();

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_PASTransmittance);
		// This does matter; transmittance is 1 outside of atmosphere.
		static const GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		cmdList.textureParameteri(lut, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		cmdList.textureParameteri(lut, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		cmdList.textureParameterfv(lut, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Render LUT

		cmdList.useProgram(program.getGLName());
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, lut, 0);
		cmdList.viewport(0, 0, LUT_WIDTH, LUT_HEIGHT);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		LOG(LogDebug, "[SkyAtmosphere] Generate transmittance LUT");
	}

}
