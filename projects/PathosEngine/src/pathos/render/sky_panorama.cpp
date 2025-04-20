#include "sky_panorama.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/gl_debug_group.h"
#include "pathos/rhi/texture.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/light_probe_baker.h"
#include "pathos/scene/sky_panorama_component.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

#include "badger/math/bits.h"
#include <string>

namespace pathos {

	static ConsoleVariable<float> cvar_panoramaSkyIntensity("r.panoramaSky.intensity", 1.0f, "Intensity boost for panorama sky");

	struct UBO_PanoramaSky {
		static constexpr uint32 BINDING_POINT = 1;

		matrix4 viewProj;
		float intensity;
	};
	
	class PanoramaSkyVS : public ShaderStage {
	public:
		PanoramaSkyVS() : ShaderStage(GL_VERTEX_SHADER, "PanoramaSkyVS") {
			addDefine("VERTEX_SHADER", 1);
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z", 1);
			}
			setFilepath("sky/sky_panorama.glsl");
		}
	};

	class PanoramaSkyFS : public ShaderStage {
	public:
		PanoramaSkyFS() : ShaderStage(GL_FRAGMENT_SHADER, "PanoramaSkyVS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("sky/sky_panorama.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_PanoramaSky, PanoramaSkyVS, PanoramaSkyFS);

}

namespace pathos {

	void PanoramaSkyPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_PanoramaSky");
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		const uint32 reflectionCubeSize = pathos::SKY_PREFILTER_MAP_DEFAULT_SIZE;
		const uint32 ambientCubeSize = pathos::SKY_AMBIENT_CUBEMAP_SIZE;

		const uint32 mipLevels = 1 + badger::ctz(reflectionCubeSize) - badger::ctz(ambientCubeSize);
		reflectionCubemap = new Texture(TextureCreateParams::cubemap(reflectionCubeSize, GL_RGBA16F, mipLevels));
		reflectionCubemap->createGPUResource_renderThread(cmdList);
		ambientCubemap = new Texture(TextureCreateParams::cubemap(ambientCubeSize, GL_RGBA16F, 1));
		ambientCubemap->createGPUResource_renderThread(cmdList);

		ubo.init<UBO_PanoramaSky>("UBO_PanoramaSky");
	}

	void PanoramaSkyPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		delete reflectionCubemap;
		delete ambientCubemap;
	}

	void PanoramaSkyPass::renderPanoramaSky(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(PanoramaSky);

		renderToScreen(cmdList, scene);
		if (scene->panoramaSky->bLightingDirty) {
			renderToCubemap(cmdList, scene);
			renderSkyDiffuseSH(cmdList);
			renderSkyPrefilterMap(cmdList);
		}
	}

	void PanoramaSkyPass::renderToScreen(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(PanoramaSkyToScreen);

		const Camera& camera = scene->camera;
		PanoramaSkyProxy* skyProxy = scene->panoramaSky;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		const matrix4 view = matrix4(matrix3(camera.getViewMatrix())); // view transform without transition
		const matrix4& proj = camera.getProjectionMatrix();

		// Write to only far plane.
		cmdList.depthFunc(GL_EQUAL);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthMask(GL_FALSE);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_PanoramaSky);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		UBO_PanoramaSky uboData;
		uboData.viewProj = proj * view;
		uboData.intensity = std::max(0.0f, cvar_panoramaSkyIntensity.getFloat());
		ubo.update(cmdList, UBO_PanoramaSky::BINDING_POINT, &uboData);

		cmdList.bindTextureUnit(0, skyProxy->texture->internal_getGLName());

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		skyProxy->sphere->bindPositionOnlyVAO(cmdList);
		skyProxy->sphere->drawPrimitive(cmdList);
	}

	void PanoramaSkyPass::renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(PanoramaToSkybox);

		GLuint panoramaTexture = scene->panoramaSky->texture->internal_getGLName();
		LightProbeBaker::get().projectPanoramaToCubemap_renderThread(
			cmdList,
			panoramaTexture,
			reflectionCubemap->internal_getGLName(),
			reflectionCubemap->getCreateParams().width);

		// Copy specular cubemap to ambient cubemap for diffuse SH.
		cmdList.generateTextureMipmap(reflectionCubemap->internal_getGLName());
		int32 copyMip = badger::ctz(reflectionCubemap->getCreateParams().width) - badger::ctz(ambientCubemap->getCreateParams().width);
		LightProbeBaker::get().copyCubemap_renderThread(cmdList, reflectionCubemap, ambientCubemap, copyMip, 0);
	}

	void PanoramaSkyPass::renderSkyDiffuseSH(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(SkyboxToDiffuseSH);
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		LightProbeBaker::get().bakeDiffuseSH_renderThread(cmdList, ambientCubemap, sceneContext.skyDiffuseSH);
	}

	void PanoramaSkyPass::renderSkyPrefilterMap(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(SkyboxToPrefilterMap);

		constexpr uint32 targetCubemapSize = pathos::SKY_PREFILTER_MAP_DEFAULT_SIZE;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		sceneContext.reallocSkyPrefilterMap(cmdList, targetCubemapSize);

		LightProbeBaker::get().bakeSpecularIBL_renderThread(
			cmdList,
			reflectionCubemap->internal_getGLName(),
			targetCubemapSize,
			sceneContext.skyPrefilterMapMipCount,
			sceneContext.skyPrefilteredMap);
	}

}
