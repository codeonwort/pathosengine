#include "skybox.h"
#include "pathos/engine_policy.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/image_based_lighting_baker.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/skybox_component.h"

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include <string>

namespace pathos {
	
	class SkyboxVS : public ShaderStage {
	public:
		SkyboxVS() : ShaderStage(GL_VERTEX_SHADER, "SkyboxVS") {
			addDefine("VERTEX_SHADER", 1);
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z", 1);
			}
			setFilepath("skybox.glsl");
		}
	};

	class SkyboxFS : public ShaderStage {
	public:
		SkyboxFS() : ShaderStage(GL_FRAGMENT_SHADER, "SkyboxFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("skybox.glsl");
		}
	};

	struct UBO_Skybox {
		matrix4 viewProj;
		float skyboxLOD;
	};

	DEFINE_SHADER_PROGRAM2(Program_Skybox, SkyboxVS, SkyboxFS);

}

namespace pathos {

	void SkyboxPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_Skybox");
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		ubo.init<UBO_Skybox>("UBO_Skybox");
	}

	void SkyboxPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void SkyboxPass::renderSkybox(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(Skybox);

		renderSkyboxToScreen(cmdList, scene);
		if (scene->sceneProxySource == SceneProxySource::MainScene && scene->skybox->bLightingDirty) {
			renderSkyIrradianceMap(cmdList, scene);
			renderSkyPreftilerMap(cmdList, scene);
		}
	}

	void SkyboxPass::renderSkyboxToScreen(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyboxToScreen);

		const Camera& camera = scene->camera;
		SkyboxProxy* skybox = scene->skybox;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_Skybox);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		UBO_Skybox uboData;
		{
			matrix4 view = matrix4(matrix3(camera.getViewMatrix())); // view transform without transition
			matrix4 proj = camera.getProjectionMatrix();
			uboData.viewProj = proj * view;
			uboData.skyboxLOD = skybox->textureLod;
		}
		ubo.update(cmdList, 1, &uboData);

		// Write to only far plane.
		cmdList.depthFunc(GL_EQUAL);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthMask(GL_FALSE);

		cmdList.cullFace(GL_FRONT);

		cmdList.bindTextureUnit(0, skybox->textureID);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		skybox->cube->activate_position(cmdList);
		skybox->cube->activateIndexBuffer(cmdList);
		skybox->cube->drawPrimitive(cmdList);

		cmdList.cullFace(GL_BACK);
	}

	void SkyboxPass::renderSkyIrradianceMap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyboxToIrradianceMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		GLuint inputCubemap = scene->skybox->textureID;
		GLuint targetCubemap = sceneContext.skyIrradianceMap;
		uint32 targetSize = pathos::SKY_IRRADIANCE_MAP_SIZE;

		ImageBasedLightingBaker::bakeSkyIrradianceMap_renderThread(
			cmdList,
			inputCubemap,
			targetCubemap,
			targetSize);
	}

	void SkyboxPass::renderSkyPreftilerMap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyboxToPrefilterMap);

		constexpr uint32 targetCubemapSize = 256; // #wip: How to determine

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		sceneContext.reallocSkyPrefilterMap(cmdList, targetCubemapSize);

		ImageBasedLightingBaker::bakeSpecularIBL_renderThread(
			cmdList,
			scene->skybox->textureID,
			targetCubemapSize,
			sceneContext.skyPrefilterMapMipCount,
			sceneContext.skyPrefilteredMap);
	}

}
