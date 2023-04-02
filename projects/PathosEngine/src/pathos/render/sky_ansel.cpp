#include "sky_ansel.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/gl_debug_group.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/image_based_lighting_baker.h"
#include "pathos/scene/sky_ansel_component.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

#include <string>

namespace pathos {

	static ConsoleVariable<float> cvar_anselSkyIntensity("r.anselSky.intensity", 1.0f, "Sky light intensity");

	static constexpr uint32 PANORAMA_TO_CUBEMAP_SIZE = 256; // #wip

	struct UBO_AnselSky {
		static constexpr uint32 BINDING_POINT = 1;

		matrix4 viewProj;
		float intensity;
	};
	
	class AnselSkyVS : public ShaderStage {
	public:
		AnselSkyVS() : ShaderStage(GL_VERTEX_SHADER, "AnselSkyVS") {
			addDefine("VERTEX_SHADER", 1);
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z", 1);
			}
			setFilepath("sky_ansel.glsl");
		}
	};

	class AnselSkyFS : public ShaderStage {
	public:
		AnselSkyFS() : ShaderStage(GL_FRAGMENT_SHADER, "AnselSkyVS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("sky_ansel.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_AnselSky, AnselSkyVS, AnselSkyFS);

}

namespace pathos {

	void AnselSkyPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_AnselSky");
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemapTexture);
		cmdList.textureStorage2D(cubemapTexture, 1, GL_RGBA16F, PANORAMA_TO_CUBEMAP_SIZE, PANORAMA_TO_CUBEMAP_SIZE);

		ubo.init<UBO_AnselSky>("UBO_AnselSky");
	}

	void AnselSkyPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		gRenderDevice->deleteTextures(1, &cubemapTexture);
	}

	void AnselSkyPass::render(RenderCommandList& cmdList, SceneProxy* scene) {
		renderToScreen(cmdList, scene);
		if (scene->sceneProxySource == SceneProxySource::MainScene) {
			renderToCubemap(cmdList, scene);
			renderSkyIrradianceMap(cmdList, scene);
			renderSkyPrefilterMap(cmdList, scene);
		}
	}

	void AnselSkyPass::renderToScreen(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(PanoramaSkyToScreen);

		const Camera& camera = scene->camera;
		AnselSkyProxy* skyProxy = scene->anselSky;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		const matrix4 view = matrix4(matrix3(camera.getViewMatrix())); // view transform without transition
		const matrix4& proj = camera.getProjectionMatrix();

		// Write to only far plane.
		cmdList.depthFunc(GL_EQUAL);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthMask(GL_FALSE);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AnselSky);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		UBO_AnselSky uboData;
		uboData.viewProj = proj * view;
		uboData.intensity = std::max(0.0f, cvar_anselSkyIntensity.getFloat());
		ubo.update(cmdList, UBO_AnselSky::BINDING_POINT, &uboData);

		cmdList.bindTextureUnit(0, skyProxy->textureID);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		skyProxy->sphere->activate_position(cmdList);
		skyProxy->sphere->activateIndexBuffer(cmdList);
		skyProxy->sphere->drawPrimitive(cmdList);
	}

	void AnselSkyPass::renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(PanoramaToSkybox);

		GLuint panoramaTexture = scene->anselSky->textureID;
		ImageBasedLightingBaker::projectPanoramaToCubemap_renderThread(
			cmdList,
			panoramaTexture,
			cubemapTexture,
			PANORAMA_TO_CUBEMAP_SIZE);
	}

	void AnselSkyPass::renderSkyIrradianceMap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyboxToIrradianceMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ImageBasedLightingBaker::bakeSkyIrradianceMap_renderThread(
			cmdList,
			cubemapTexture,
			sceneContext.skyIrradianceMap,
			pathos::SKY_IRRADIANCE_MAP_SIZE);
	}

	void AnselSkyPass::renderSkyPrefilterMap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyboxToPrefilterMap);

		constexpr uint32 targetCubemapSize = 256; // #wip: How to determine

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		sceneContext.reallocSkyPrefilterMap(cmdList, targetCubemapSize);

		ImageBasedLightingBaker::bakeSpecularIBL_renderThread(
			cmdList,
			cubemapTexture,
			targetCubemapSize,
			sceneContext.skyPrefilterMapMipCount,
			sceneContext.skyPrefilteredMap);
	}

}
