#include "sky_atmosphere.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/light_probe_baker.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/sky_common.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/scene/sky_atmosphere_component.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/engine_util.h"
#include "pathos/util/log.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

#include "badger/math/bits.h"

// Precomputed Atmospheric Scattering
namespace pathos {

	static ConsoleVariable<float> cvar_sunDisk("r.atmosphere.sunDiskSize", 5.0f, "Sun disk size");

	static constexpr uint32 LUT_WIDTH = 64;
	static constexpr uint32 LUT_HEIGHT = 256;

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

		// To copy to ambientCubemap.
		reflectionCubemap = new Texture(TextureCreateParams::cubemap(pathos::SKY_PREFILTER_MAP_SIZE, GL_RGBA16F, pathos::SKY_PREFILTER_MAP_MIP_COUNT));
		reflectionCubemap->createGPUResource_renderThread(cmdList);

		ambientCubemap = new Texture(TextureCreateParams::cubemap(pathos::SKY_AMBIENT_CUBEMAP_SIZE, GL_RGBA16F, 1));
		ambientCubemap->createGPUResource_renderThread(cmdList);

		ubo.init<UBO_Atmosphere>("UBO_SkyAtmosphere");
		gRenderDevice->createVertexArrays(1, &vao);
	}

	void SkyAtmospherePass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		delete reflectionCubemap;
		delete ambientCubemap;
		gRenderDevice->deleteVertexArrays(1, &vao);
		if (transmittanceLUT) delete transmittanceLUT;
		ubo.safeDestroy();
	}

	void SkyAtmospherePass::renderSkyAtmosphere(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera, MeshGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(SkyAtmosphere);

		renderTransmittanceLUT(cmdList, fullscreenQuad);
		renderToScreen(cmdList, scene, camera); // #todo-sky: Too slow (2.23027 ms for 1920x1080)

		if (scene->skyAtmosphere->bLightingDirty) {
			const ESkyLightingUpdateMode mode = scene->skyAtmosphere->lightingMode;
			const ESkyLightingUpdatePhase phase = scene->skyAtmosphere->lightingPhase;
			if (mode == ESkyLightingUpdateMode::EveryFrame) {
				renderToCubemap(cmdList, scene, 0, 5);
				generateCubemapMips(cmdList);
				computeDiffuseSH(cmdList);
				filterSpecular(cmdList);
			} else if (mode == ESkyLightingUpdateMode::Progressive) {
				if (phase == ESkyLightingUpdatePhase::RenderFacePosX) renderToCubemap(cmdList, scene, 0, 0);
				if (phase == ESkyLightingUpdatePhase::RenderFaceNegX) renderToCubemap(cmdList, scene, 1, 1);
				if (phase == ESkyLightingUpdatePhase::RenderFacePosY) renderToCubemap(cmdList, scene, 2, 2);
				if (phase == ESkyLightingUpdatePhase::RenderFaceNegY) renderToCubemap(cmdList, scene, 3, 3);
				if (phase == ESkyLightingUpdatePhase::RenderFacePosZ) renderToCubemap(cmdList, scene, 4, 4);
				if (phase == ESkyLightingUpdatePhase::RenderFaceNegZ) renderToCubemap(cmdList, scene, 5, 5);
				if (phase == ESkyLightingUpdatePhase::GenerateMips) generateCubemapMips(cmdList);
				if (phase == ESkyLightingUpdatePhase::DiffuseSH) computeDiffuseSH(cmdList);
				if (phase == ESkyLightingUpdatePhase::SpecularFilter) filterSpecular(cmdList);
			}
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

	void SkyAtmospherePass::renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene, int32 faceBegin, int32 faceEnd) {
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

		// Render specular cubemap.

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AtmosphericScattering);
		cmdList.useProgram(program.getGLName());

		cmdList.disable(GL_DEPTH_TEST);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.viewport(0, 0, pathos::SKY_PREFILTER_MAP_SIZE, pathos::SKY_PREFILTER_MAP_SIZE);

		cmdList.bindTextureUnit(0, transmittanceLUT->internal_getGLName());
		cmdList.bindVertexArray(vao);

		for (int32 i = faceBegin; i <= faceEnd; ++i) {
			tempCamera.lookAt(vector3(0.0f), lookAtOffsets[i], upVectors[i]);
			bool flipScreenXY = (i != 2 && i != 3);

			UBO_Atmosphere uboData;
			uboData.sunIntensity    = sunIntensity;
			uboData.sunDiskSize     = sunDiskSize;
			uboData.screenFlip.x    = flipScreenXY ? -1.0f : 1.0f;
			uboData.screenFlip.y    = flipScreenXY ? -1.0f : 1.0f;
			uboData.renderToCubemap = 1;
			uboData.cubemapSize     = (float)pathos::SKY_PREFILTER_MAP_SIZE;
			uboData.customViewInv   = glm::inverse(tempCamera.getViewMatrix());
			ubo.update(cmdList, UBO_Atmosphere::BINDING_POINT, &uboData);

			cmdList.namedFramebufferTextureLayer(fbo, GL_COLOR_ATTACHMENT0, reflectionCubemap->internal_getGLName(), 0, i);
			cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		cmdList.bindVertexArray(0);
	}

	void SkyAtmospherePass::generateCubemapMips(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(SkyAtmosphereMips);

		cmdList.generateTextureMipmap(reflectionCubemap->internal_getGLName());

		// Copy specular cubemap to ambient cubemap for diffuse SH.
		int32 copyMip = badger::ctz(pathos::SKY_PREFILTER_MAP_SIZE) - badger::ctz(pathos::SKY_AMBIENT_CUBEMAP_SIZE);
		LightProbeBaker::get().copyCubemap_renderThread(cmdList, reflectionCubemap, ambientCubemap, copyMip, 0);
	}

	void SkyAtmospherePass::computeDiffuseSH(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(SkyDiffuseSH);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		LightProbeBaker::get().bakeDiffuseSH_renderThread(cmdList, ambientCubemap, sceneContext.skyDiffuseSH);
	}

	void SkyAtmospherePass::filterSpecular(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(SkyPrefilterSpecular);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		sceneContext.reallocSkyPrefilterMap(cmdList, pathos::SKY_PREFILTER_MAP_SIZE);
		LightProbeBaker::get().bakeReflectionProbe_renderThread(cmdList, reflectionCubemap->internal_getGLName(), sceneContext.skyPrefilteredMap);
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
		transmittanceLUT->createGPUResource_renderThread(cmdList);

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
