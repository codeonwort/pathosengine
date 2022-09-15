#include "render_deferred.h"

#include "pathos/engine_policy.h"

#include "pathos/render/render_device.h"
#include "pathos/render/depth_prepass.h"
#include "pathos/render/sky.h"
#include "pathos/render/skybox.h"
#include "pathos/render/sky_ansel.h"
#include "pathos/render/sky_atmosphere.h"
#include "pathos/render/sky_clouds.h"
#include "pathos/render/god_ray.h"
#include "pathos/render/visualize_buffer.h"
#include "pathos/render/render_target.h"
#include "pathos/render/forward/translucency_rendering.h"
#include "pathos/render/postprocessing/ssao.h"
#include "pathos/render/postprocessing/bloom_setup.h"
#include "pathos/render/postprocessing/bloom.h"
#include "pathos/render/postprocessing/tone_mapping.h"
#include "pathos/render/postprocessing/depth_of_field.h"
#include "pathos/render/postprocessing/anti_aliasing_fxaa.h"

#include "pathos/light/directional_light_component.h"
#include "pathos/light/point_light_component.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/mesh/mesh.h"
#include "pathos/texture/volume_texture.h"
#include "pathos/shader/shader_program.h"

#include "pathos/console.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/gl_debug_group.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	class CopyTextureVS : public ShaderStage {
	public:
		CopyTextureVS() : ShaderStage(GL_VERTEX_SHADER, "CopyTextureVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class CopyTextureFS : public ShaderStage {
	public:
		CopyTextureFS() : ShaderStage(GL_FRAGMENT_SHADER, "CopyTextureFS")
		{
			setFilepath("copy_texture.fs.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_CopyTexture, CopyTextureVS, CopyTextureFS);

}

namespace pathos {

	static ConsoleVariable<int32> cvar_frustum_culling("r.frustum_culling", 1, "0 = disable, 1 = enable");
	static ConsoleVariable<int32> cvar_enable_ssr("r.ssr.enable", 1, "0 = disable SSR, 1 = enable SSR");
	static ConsoleVariable<int32> cvar_enable_bloom("r.bloom", 1, "0 = disable bloom, 1 = enable bloom");
	static ConsoleVariable<int32> cvar_enable_dof("r.dof.enable", 1, "0 = disable DoF, 1 = enable DoF");
	static ConsoleVariable<int32> cvar_anti_aliasing("r.antialiasing.method", 1, "0 = disable, 1 = FXAA");

	static constexpr uint32 MAX_DIRECTIONAL_LIGHTS        = 4;
	static constexpr uint32 MAX_POINT_LIGHTS              = 8;

	struct UBO_PerFrame {
		matrix4               prevView; // For reprojection
		matrix4               prevInverseView; // For reprojection
		matrix4               view;
		matrix4               inverseView;
		matrix3x4             view3x3; // Name is 3x3, but type should be 3x4 due to how padding works in glsl
		matrix4               viewProj;
		matrix4               proj;
		matrix4               inverseProj;

		vector4               projParams;
		vector4               screenResolution; // (w, h, 1/w, 1/h)
		vector4               zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))
		vector4               time; // (currentTime, ?, ?, ?)

		matrix4               sunViewProj[4];

		vector3               eyeDirection;
		float                 __pad0;

		vector3               eyePosition;
		float                 __pad1;

		vector3               ws_eyePosition;
		uint32                numDirLights;

		DirectionalLightProxy directionalLights[MAX_DIRECTIONAL_LIGHTS];

		uint32                numPointLights;
		vector3               __pad2;

		PointLightProxy       pointLights[MAX_POINT_LIGHTS];
	};
	static constexpr GLuint SCENE_UNIFORM_BINDING_INDEX = 0;

	DeferredRenderer::DeferredRenderer()
		: antiAliasing(EAntiAliasingMethod::FXAA)
		, scene(nullptr)
		, camera(nullptr)
	{
		sceneRenderTargets.useGBuffer = true;
	}

	DeferredRenderer::~DeferredRenderer() {
		CHECK(destroyed);
	}

	void DeferredRenderer::initializeResources(RenderCommandList& cmdList) {
		sceneRenderTargets.reallocSceneTextures(cmdList, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight);
		cmdList.flushAllCommands();
		cmdList.sceneRenderTargets = &sceneRenderTargets;
	}

	void DeferredRenderer::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			destroySceneRenderTargets(cmdList);
		}
		destroyed = true;
	}

	void DeferredRenderer::setSceneRenderSettings(const SceneRenderSettings& settings) {
		CHECK(settings.isValid());

		sceneRenderSettings = settings;
		frameCounter = sceneRenderSettings.frameCounter; // #todo: Duplicate with SceneProxy::frameNumber
		
		if (settings.finalRenderTarget != nullptr) {
			setFinalRenderTarget(settings.finalRenderTarget);
		}
	}

	void DeferredRenderer::setFinalRenderTarget(RenderTarget2D* inFinalRenderTarget) {
		CHECKF(inFinalRenderTarget != nullptr, "null is not accepted. Use setFinalRenderTargetToBackbuffer() for backbuffer");
		CHECKF(inFinalRenderTarget->isDepthFormat() == false, "Depth format is not supported yet");
		finalRenderTarget = inFinalRenderTarget;
	}

	void DeferredRenderer::setFinalRenderTargetToBackbuffer() {
		finalRenderTarget = 0;
	}

	void DeferredRenderer::reallocateSceneRenderTargets(RenderCommandList& cmdList) {
		sceneRenderTargets.reallocSceneTextures(cmdList, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight);

		if (gbufferFBO == 0) {
			gRenderDevice->createFramebuffers(1, &gbufferFBO);
			cmdList.objectLabel(GL_FRAMEBUFFER, gbufferFBO, -1, "FBO_gbuffer");
		}
		// Do this everytime as reallocSceneTextures() might recreate GL textures.
		{
			GLenum gbuffer_draw_buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT0, sceneRenderTargets.gbufferA, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT1, sceneRenderTargets.gbufferB, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT2, sceneRenderTargets.gbufferC, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_DEPTH_ATTACHMENT, sceneRenderTargets.sceneDepth, 0);
			cmdList.namedFramebufferDrawBuffers(gbufferFBO, 3, gbuffer_draw_buffers);

			pathos::checkFramebufferStatus(cmdList, gbufferFBO, "gbuffer setup is invalid");
		}
	}

	void DeferredRenderer::destroySceneRenderTargets(RenderCommandList& cmdList) {
		sceneRenderTargets.freeSceneTextures(cmdList);
		gRenderDevice->deleteFramebuffers(1, &gbufferFBO);
	}

	void DeferredRenderer::render(RenderCommandList& cmdList, SceneProxy* inScene, Camera* inCamera) {
		scene = inScene;
		camera = inCamera;

		CHECK(sceneRenderSettings.isValid());

		cmdList.sceneProxy = inScene;
		cmdList.sceneRenderTargets = &sceneRenderTargets;
		reallocateSceneRenderTargets(cmdList);

		// #todo: multi-view
		{
			SCOPED_CPU_COUNTER(UpdateUniformBuffer);

			// They should be updated before updateSceneUniformBuffer
			scene->createViewDependentRenderProxy(camera->getViewMatrix());
			sunShadowMap->updateUniformBufferData(cmdList, scene, camera);

			// Update ubo_perFrame
			updateSceneUniformBuffer(cmdList, scene, camera);
		}

		if (cvar_frustum_culling.getInt() != 0) {
			scene->checkFrustumCulling(*camera);
		}

		if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
			cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		}

		{
			SCOPED_CPU_COUNTER(RenderPreDepth);
			SCOPED_GPU_COUNTER(RenderPreDepth);

			depthPrepass->renderPreDepth(cmdList, scene, camera);
		}

		{
			SCOPED_CPU_COUNTER(RenderCascadedShadowMap);
			SCOPED_GPU_COUNTER(RenderCascadedShadowMap);
			// #todo-performance: This is incredibly slow in debug build
			sunShadowMap->renderShadowMap(cmdList, scene, camera);
		}

		{
			SCOPED_CPU_COUNTER(RenderOmniShadowMaps);
			SCOPED_GPU_COUNTER(RenderOmniShadowMaps);
			// #todo-performance: This is incredibly super slow in debug build
			omniShadowPass->renderShadowMaps(cmdList, scene, camera);
		}

		// #todo-renderer: Why am I clearing backbuffer here?
		{
			SCOPED_DRAW_EVENT(ClearBackbuffer);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			cmdList.clearColor(0.0f, 0.0f, 0.0f, 0.0f);
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				cmdList.clearDepth(0.0f);
			} else {
				cmdList.clearDepth(1.0f);
			}
			cmdList.clearStencil(0);
			cmdList.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}

		// Volumetric clouds
		{
			SCOPED_GPU_COUNTER(VolumetricCloudPass);
			volumetricCloud->renderVolumetricCloud(cmdList, scene);
		}

		// GodRay
		// input: static meshes
		// output: god ray texture
		{
			SCOPED_GPU_COUNTER(RenderGodRay);

			godRay->renderGodRay(cmdList, scene, camera, fullscreenQuad.get(), this);
		}

		// #todo-refactoring: Rename to renderBasePass
		{
			SCOPED_GPU_COUNTER(BasePass);
			renderBasePass(cmdList);
		}

		{
			SCOPED_GPU_COUNTER(SSAO);
			ssao->renderPostProcess(cmdList, fullscreenQuad.get());
		}

		{
			SCOPED_DRAW_EVENT(ClearSceneColor);
			GLfloat* clearValues = (GLfloat*)cmdList.allocateSingleFrameMemory(4 * sizeof(GLfloat));
			for (int32 i = 0; i < 4; ++i) clearValues[i] = 0.0f;
			cmdList.clearTexImage(
				sceneRenderTargets.sceneColor,
				0,
				GL_RGBA,
				GL_FLOAT,
				clearValues);
		}

		{
			// #todo-gpu-counter: Sky rendering cost should not be included here (or support nested counters)
			SCOPED_GPU_COUNTER(DirectLighting);
			renderDirectLighting(cmdList);
		}

		{
			SCOPED_GPU_COUNTER(IndirectLighting);

			indirectLightingPass->renderIndirectLighting(cmdList, scene, camera, fullscreenQuad.get());
		}

		if (cvar_enable_ssr.getInt() != 0) {
			SCOPED_GPU_COUNTER(ScreenSpaceReflection);
			screenSpaceReflectionPass->renderScreenSpaceReflection(cmdList, scene, camera, fullscreenQuad.get());
		}

		// Translucency pass
		{
			SCOPED_GPU_COUNTER(Translucency);

			renderTranslucency(cmdList);
		}

		//////////////////////////////////////////////////////////////////////////
		// Post-processing
		if (sceneRenderSettings.enablePostProcess == false)
		{
			copyTexture(cmdList, sceneRenderTargets.sceneColor, getFinalRenderTarget());
		}
		else
		{
			// #todo: Support nested gpu counters
			SCOPED_GPU_COUNTER(PostProcessing);

			antiAliasing = (EAntiAliasingMethod)pathos::max(0, pathos::min((int32)EAntiAliasingMethod::NumMethods, cvar_anti_aliasing.getInt()));

			const bool noAA = (antiAliasing == EAntiAliasingMethod::NoAA);
			const bool noBloom = (cvar_enable_bloom.getInt() == 0);
			const bool noDOF = (cvar_enable_dof.getInt() == 0) || (depthOfField->isAvailable() == false);

			GLuint sceneAfterLastPP = sceneRenderTargets.sceneColor;

			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);

			// Downsample SceneColor to generate mipmaps. Only used by bloom pass for now.
			if (!noBloom) {
				SCOPED_DRAW_EVENT(SceneColorDownsample);

				uint32 viewportWidth = sceneRenderSettings.sceneWidth / 2;
				uint32 viewportHeight = sceneRenderSettings.sceneHeight / 2;

				bloomSetup->setInput(EPostProcessInput::PPI_0, sceneRenderTargets.sceneColor);
				bloomSetup->setOutput(EPostProcessOutput::PPO_0, sceneRenderTargets.sceneBloom); // temp use for downsample
				bloomSetup->renderPostProcess(cmdList, fullscreenQuad.get());

				const GLuint firstSource = sceneRenderTargets.sceneBloom;
				const uint32 numMips = sceneRenderTargets.sceneColorDownsampleMipmapCount;

				for (uint32 i = 0; i < numMips; ++i) {
					const GLuint source = i == 0 ? firstSource : sceneRenderTargets.sceneColorDownsampleViews[i - 1];
					const GLuint target = sceneRenderTargets.sceneColorDownsampleViews[i];
					cmdList.viewport(0, 0, viewportWidth, viewportHeight);
					copyTexture(cmdList, source, target);
					viewportWidth /= 2;
					viewportHeight /= 2;
				}
			}

			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);

			// Post Process: Bloom
			{
				cmdList.viewport(0, 0, sceneRenderSettings.sceneWidth / 2, sceneRenderSettings.sceneHeight / 2);

				bloomSetup->setInput(EPostProcessInput::PPI_0, sceneRenderTargets.sceneColor);
				bloomSetup->setOutput(EPostProcessOutput::PPO_0, sceneRenderTargets.sceneBloom);
				if (noBloom) {
					bloomSetup->clearSceneBloom(cmdList, fullscreenQuad.get());
				} else {
					// #todo-bloom: Inputs are not used anymore (Cannot pass texture views by setInput())
					bloomPass->setInput(EPostProcessInput::PPI_0, sceneRenderTargets.sceneBloom);
					bloomPass->setInput(EPostProcessInput::PPI_1, sceneRenderTargets.sceneBloomTemp);
					bloomPass->renderPostProcess(cmdList, fullscreenQuad.get());
				}

				cmdList.viewport(0, 0, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight);
			}

			// Post Process: Tone Mapping
			{
				// #todo-postprocess: How to check if current PP is the last? (standard way is needed, not ad-hoc like this)
				const bool isFinalPP = (noAA && noDOF);

				toneMapping->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
				toneMapping->setInput(EPostProcessInput::PPI_1, sceneRenderTargets.sceneBloom);
				toneMapping->setInput(EPostProcessInput::PPI_2, sceneRenderTargets.godRayResult);
				toneMapping->setInput(EPostProcessInput::PPI_3, sceneRenderTargets.getVolumetricCloud(frameCounter));
				toneMapping->setOutput(EPostProcessOutput::PPO_0, isFinalPP ? getFinalRenderTarget() : sceneRenderTargets.toneMappingResult);
				toneMapping->renderPostProcess(cmdList, fullscreenQuad.get());

				sceneAfterLastPP = sceneRenderTargets.toneMappingResult;
			}

			// Post Process: Anti-aliasing
			switch (antiAliasing) {
			case EAntiAliasingMethod::NoAA:
				// Do nothing
				break;

			case EAntiAliasingMethod::FXAA:
				{
					// #todo-postprocess: How to check if current PP is the last? (standard way is needed, not ad-hoc like this)
					const bool isFinalPP = noDOF;

					fxaa->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
					fxaa->setOutput(EPostProcessOutput::PPO_0, isFinalPP ? getFinalRenderTarget() : sceneRenderTargets.sceneFinal);
					fxaa->renderPostProcess(cmdList, fullscreenQuad.get());

					sceneAfterLastPP = sceneRenderTargets.sceneFinal;
				}
				break;

			default:
				break;
			}

			// Post Process: Depth of Field
			if (!noDOF) {
				const GLuint dofRenderTarget = getFinalRenderTarget();

				depthOfField->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
				depthOfField->setOutput(EPostProcessOutput::PPO_0, dofRenderTarget);
				depthOfField->renderPostProcess(cmdList, fullscreenQuad.get());
			}

		}

		// Debug pass (r.viewmode)
		visualizeBuffer->render(cmdList, scene, camera);

		scene = nullptr;
		camera = nullptr;
	}

	void DeferredRenderer::renderBasePass(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(BasePass);

		// #todo: Dynamically toggle depth prepass.
		constexpr bool bUseDepthPrepass = true;
		constexpr bool bReverseZ = pathos::getReverseZPolicy() == EReverseZPolicy::Reverse;

		static const GLuint color_zero_ui[] = { 0, 0, 0, 0 };
		static const GLfloat color_zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat* sceneDepthClearValue = (GLfloat*)cmdList.allocateSingleFrameMemory(sizeof(GLfloat));
		*sceneDepthClearValue = pathos::getDeviceFarDepth();

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, gbufferFBO);
		cmdList.clearNamedFramebufferuiv(gbufferFBO, GL_COLOR, 0, color_zero_ui);
		cmdList.clearNamedFramebufferfv(gbufferFBO, GL_COLOR, 1, color_zero);
		cmdList.clearNamedFramebufferfv(gbufferFBO, GL_COLOR, 2, color_zero);
		if (!bUseDepthPrepass) {
			cmdList.clearNamedFramebufferfv(gbufferFBO, GL_DEPTH, 0, sceneDepthClearValue);
		}

		// Set render state
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, gbufferFBO);
		cmdList.viewport(0, 0, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight);

		if (bUseDepthPrepass) {
			cmdList.depthFunc(bReverseZ ? GL_GEQUAL : GL_LEQUAL);
			cmdList.enable(GL_DEPTH_TEST);
			cmdList.depthMask(GL_FALSE);
		} else {
			cmdList.depthFunc(bReverseZ ? GL_GREATER : GL_LESS);
			cmdList.enable(GL_DEPTH_TEST);
			cmdList.depthMask(GL_TRUE);
		}

		bool bEnableFrustumCulling = cvar_frustum_culling.getInt() != 0;

		const uint8 numMaterialIDs = (uint8)MATERIAL_ID::NUM_MATERIAL_IDS;
		for (uint8 i = 0; i < numMaterialIDs; ++i) {
			MeshDeferredRenderPass_Pack* pass = pack_passes[i];

			bool fallbackPass = false;
			if (pass == nullptr) {
				pass = pack_passes[static_cast<uint16>(MATERIAL_ID::SOLID_COLOR)];
				fallbackPass = true;
			}

			if (i == (uint8)MATERIAL_ID::TRANSLUCENT_SOLID_COLOR) {
				// Translucency is not rendered in gbuffer pass
				continue;
			}

			const auto& proxyList = scene->proxyList_staticMesh[i];

			if (proxyList.size() == 0) {
				continue;
			}

			{
				SCOPED_DRAW_EVENT(BindMaterialProgram);
				pass->bindProgram(cmdList);
			}

			for (auto j = 0u; j < proxyList.size(); ++j) {
				const StaticMeshProxy& item = *(proxyList[j]);
				Material* materialOverride = fallbackPass ? fallbackMaterial.get() : item.material;

				if (bEnableFrustumCulling && !item.bInFrustum) {
					continue;
				}

				// #todo-renderer: Batching by same state
				if (item.doubleSided) cmdList.disable(GL_CULL_FACE);
				if (item.renderInternal) cmdList.frontFace(GL_CW);

 				pass->setModelMatrix(item.modelMatrix);
 				pass->render(cmdList, scene, camera, item.geometry, materialOverride);

				// #todo-renderer: Batching by same state
				if (item.doubleSided) cmdList.enable(GL_CULL_FACE);
				if (item.renderInternal) cmdList.frontFace(GL_CCW);
			}
		}

		cmdList.depthMask(GL_TRUE);
	}

	void DeferredRenderer::renderDirectLighting(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(DirectLighting);

		directLightingPass->bindFramebuffer(cmdList);

		// #todo-refactoring: Actually not an unpack work, but rendering order is here
		const bool bRenderSkybox = scene->isSkyboxValid();
		const bool bRenderAnsel = scene->isAnselSkyValid();
		const bool bRenderAtmosphere = scene->isSkyAtmosphereValid();
		{
			// #todo-sky: What to choose when multiple sky proxies are active?
			//int32 numActiveSkies = (int32)bRenderSkybox + (int32)bRenderAnsel + (int32)bRenderAtmosphere;
			//CHECKF(numActiveSkies <= 1, "At most one sky representation is allowed at the same time");
		}
		if (scene->isSkyboxValid()) {
			skyboxPass->render(cmdList, scene);
		} else if (scene->isAnselSkyValid()) {
			anselSkyPass->render(cmdList, scene);
		} else if (scene->isSkyAtmosphereValid()) {
			skyAtmospherePass->render(cmdList, scene);
		}
		
		directLightingPass->renderDirectLighting(cmdList, scene, camera);
	}
	
	// #todo-translucency: Implement
	void DeferredRenderer::renderTranslucency(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(Translucency);

		uint8 materialID = (uint8)MATERIAL_ID::TRANSLUCENT_SOLID_COLOR;
		const auto& meshBatches = scene->proxyList_staticMesh[materialID];

		translucency_pass->renderTranslucency(cmdList, camera, meshBatches);
	}

	void DeferredRenderer::copyTexture(RenderCommandList& cmdList, GLuint source, GLuint target) {
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_CopyTexture);

		cmdList.useProgram(program.getGLName());
		if (target == 0) {
			cmdList.bindFramebuffer(GL_FRAMEBUFFER, 0);
		} else {
			cmdList.bindFramebuffer(GL_FRAMEBUFFER, DeferredRenderer::copyTextureFBO);
			cmdList.namedFramebufferTexture(DeferredRenderer::copyTextureFBO, GL_COLOR_ATTACHMENT0, target, 0);
		}
		cmdList.bindTextureUnit(0, source);
		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
		fullscreenQuad->deactivate(cmdList);
		fullscreenQuad->deactivateIndexBuffer(cmdList);
	}

	GLuint DeferredRenderer::getFinalRenderTarget() const {
		if (finalRenderTarget == nullptr) {
			return 0; // Default backbuffer
		}
		return finalRenderTarget->getGLName();
	}

	void DeferredRenderer::updateSceneUniformBuffer(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) {
		UBO_PerFrame data;

		const matrix4& projMatrix = camera->getProjectionMatrix();

		if (frameCounter != 0) {
			data.prevView        = prevView;
			data.prevInverseView = prevInverseView;
		} else {
			data.prevView        = matrix4(1.0f);
			data.prevInverseView = matrix4(1.0f);
		}
		data.view         = camera->getViewMatrix();
		data.inverseView  = glm::inverse(data.view);
		data.view3x3      = matrix3x4(data.view);
		data.viewProj     = camera->getViewProjectionMatrix();
		data.proj         = projMatrix;
		data.inverseProj  = glm::inverse(projMatrix);

		data.projParams  = vector4(1.0f / projMatrix[0][0], 1.0f / projMatrix[1][1], 0.0f, 0.0f);

		data.screenResolution.x = (float)sceneRenderSettings.sceneWidth;
		data.screenResolution.y = (float)sceneRenderSettings.sceneHeight;
		data.screenResolution.z = 1.0f / data.screenResolution.x;
		data.screenResolution.w = 1.0f / data.screenResolution.y;

		data.zRange.x = camera->getZNear();
		data.zRange.y = camera->getZFar();
		data.zRange.z = camera->getFovYRadians();
		data.zRange.w = camera->getAspectRatio();

		data.time        = vector4(gEngine->getWorldTime(), 0.0, 0.0, 0.0);

		data.sunViewProj[0] = sunShadowMap->getViewProjection(0);
		data.sunViewProj[1] = sunShadowMap->getViewProjection(1);
		data.sunViewProj[2] = sunShadowMap->getViewProjection(2);
		data.sunViewProj[3] = sunShadowMap->getViewProjection(3);

		data.eyeDirection = vector3(camera->getViewMatrix() * vector4(camera->getEyeVector(), 0.0f));
		data.eyePosition  = vector3(camera->getViewMatrix() * vector4(camera->getPosition(), 1.0f));

		data.ws_eyePosition = camera->getPosition();

		data.numDirLights = pathos::min((uint32)scene->proxyList_directionalLight.size(), MAX_DIRECTIONAL_LIGHTS);
		for (uint32 i = 0; i < data.numDirLights; ++i) {
			data.directionalLights[i] = *(scene->proxyList_directionalLight[i]);
		}

		data.numPointLights = pathos::min((uint32)scene->proxyList_pointLight.size(), MAX_POINT_LIGHTS);
		for (uint32 i = 0; i < data.numPointLights; ++i) {
			data.pointLights[i] = *(scene->proxyList_pointLight[i]);
		}

		ubo_perFrame->update(cmdList, SCENE_UNIFORM_BINDING_INDEX, &data);

		prevView = data.view;
		prevInverseView = data.inverseView;
	}

}

namespace pathos {
	
	std::unique_ptr<class ColorMaterial>           DeferredRenderer::fallbackMaterial;
	std::unique_ptr<class PlaneGeometry>           DeferredRenderer::fullscreenQuad;
	GLuint                                         DeferredRenderer::copyTextureFBO = 0;
	
	std::unique_ptr<UniformBuffer>                 DeferredRenderer::ubo_perFrame;
	
	MeshDeferredRenderPass_Pack*                   DeferredRenderer::pack_passes[static_cast<uint32>(MATERIAL_ID::NUM_MATERIAL_IDS)];
	
	std::unique_ptr<DirectLightingPass>            DeferredRenderer::directLightingPass;
	std::unique_ptr<IndirectLightingPass>          DeferredRenderer::indirectLightingPass;
	std::unique_ptr<ScreenSpaceReflectionPass>     DeferredRenderer::screenSpaceReflectionPass;

	std::unique_ptr<class TranslucencyRendering>   DeferredRenderer::translucency_pass;

	std::unique_ptr<class SkyboxPass>              DeferredRenderer::skyboxPass;
	std::unique_ptr<class AnselSkyPass>            DeferredRenderer::anselSkyPass;
	std::unique_ptr<class SkyAtmospherePass>       DeferredRenderer::skyAtmospherePass;
	std::unique_ptr<class VolumetricCloudPass>     DeferredRenderer::volumetricCloud;

	std::unique_ptr<class DepthPrepass>            DeferredRenderer::depthPrepass;
	std::unique_ptr<DirectionalShadowMap>          DeferredRenderer::sunShadowMap;
	std::unique_ptr<OmniShadowPass>                DeferredRenderer::omniShadowPass;
	std::unique_ptr<class VisualizeBufferPass>     DeferredRenderer::visualizeBuffer;

	std::unique_ptr<class GodRay>                  DeferredRenderer::godRay;
	std::unique_ptr<class SSAO>                    DeferredRenderer::ssao;
	std::unique_ptr<class BloomSetup>              DeferredRenderer::bloomSetup;
	std::unique_ptr<class BloomPass>               DeferredRenderer::bloomPass;
	std::unique_ptr<class ToneMapping>             DeferredRenderer::toneMapping;
	std::unique_ptr<class FXAA>                    DeferredRenderer::fxaa;
	std::unique_ptr<class DepthOfField>            DeferredRenderer::depthOfField;

	void DeferredRenderer::internal_initGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		fallbackMaterial = std::make_unique<ColorMaterial>();
		fallbackMaterial->setAlbedo(1.0f, 0.4f, 0.7f);
		fallbackMaterial->setMetallic(0.0f);
		fallbackMaterial->setRoughness(0.0f);

		fullscreenQuad = std::make_unique<PlaneGeometry>(2.0f, 2.0f);
		gRenderDevice->createFramebuffers(1, &copyTextureFBO);
		cmdList.namedFramebufferDrawBuffer(copyTextureFBO, GL_COLOR_ATTACHMENT0);

		ubo_perFrame = std::make_unique<UniformBuffer>();
		ubo_perFrame->init<UBO_PerFrame>("UBO_PerFrame");

		const int32 uboMaxSize = gRenderDevice->getCapabilities().glMaxUniformBlockSize;
		if (sizeof(UBO_PerFrame) > uboMaxSize) {
			char msg[256];
			sprintf_s(msg, "%s: UBO_PerFrame is bigger than the device's limit (%d > %d)",
				__FUNCTION__, (int32)sizeof(UBO_PerFrame), uboMaxSize);
			CHECKF(false, msg);
		}

		{
			for (uint8 i = 0; i < (uint8)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
				pack_passes[i] = nullptr;
			}
			pack_passes[(uint8)MATERIAL_ID::SOLID_COLOR] = new MeshDeferredRenderPass_Pack_SolidColor;
			pack_passes[(uint8)MATERIAL_ID::FLAT_TEXTURE] = new MeshDeferredRenderPass_Pack_FlatTexture;
			pack_passes[(uint8)MATERIAL_ID::WIREFRAME] = new MeshDeferredRenderPass_Pack_Wireframe;
			pack_passes[(uint8)MATERIAL_ID::BUMP_TEXTURE] = new MeshDeferredRenderPass_Pack_BumpTexture;
			pack_passes[(uint8)MATERIAL_ID::ALPHA_ONLY_TEXTURE] = new MeshDeferredRenderPass_Pack_AlphaOnly;
			pack_passes[(uint8)MATERIAL_ID::PBR_TEXTURE] = new MeshDeferredRenderPass_Pack_PBR;
			for (uint8 i = 0; i < (uint8)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
				if (pack_passes[i] == nullptr) {
					LOG(LogWarning, "BasePass not present for material id: %u", i);
				}
			}

			directLightingPass = std::make_unique<DirectLightingPass>();
			indirectLightingPass = std::make_unique<IndirectLightingPass>();
			screenSpaceReflectionPass = std::make_unique<ScreenSpaceReflectionPass>();
			translucency_pass = std::make_unique<TranslucencyRendering>();

			directLightingPass->initializeResources(cmdList);
			indirectLightingPass->initializeResources(cmdList);
			screenSpaceReflectionPass->initializeResources(cmdList);
			translucency_pass->initializeResources(cmdList);
		}

		{
			skyboxPass = std::make_unique<SkyboxPass>();
			skyboxPass->initializeResources(cmdList);

			anselSkyPass = std::make_unique<AnselSkyPass>();
			anselSkyPass->initializeResources(cmdList);

			skyAtmospherePass = std::make_unique<SkyAtmospherePass>();
			skyAtmospherePass->initializeResources(cmdList);

			volumetricCloud = std::make_unique<VolumetricCloudPass>();
			volumetricCloud->initializeResources(cmdList);
		}

		{
			depthPrepass = std::make_unique<DepthPrepass>();
			sunShadowMap = std::make_unique<DirectionalShadowMap>();
			omniShadowPass = std::make_unique<OmniShadowPass>();
			visualizeBuffer = std::make_unique<VisualizeBufferPass>();

			depthPrepass->initializeResources(cmdList);
			sunShadowMap->initializeResources(cmdList);
			omniShadowPass->initializeResources(cmdList);
			visualizeBuffer->initializeResources(cmdList);
		}

		{
			godRay = std::make_unique<GodRay>();
			ssao = std::make_unique<SSAO>();
			bloomSetup = std::make_unique<BloomSetup>();
			bloomPass = std::make_unique<BloomPass>();
			toneMapping = std::make_unique<ToneMapping>();
			fxaa = std::make_unique<FXAA>();
			depthOfField = std::make_unique<DepthOfField>();

			godRay->initializeResources(cmdList);
			ssao->initializeResources(cmdList);
			bloomSetup->initializeResources(cmdList);
			bloomPass->initializeResources(cmdList);
			toneMapping->initializeResources(cmdList);
			fxaa->initializeResources(cmdList);
			depthOfField->initializeResources(cmdList);
		}
	}

	void DeferredRenderer::internal_destroyGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		fallbackMaterial.release();
		fullscreenQuad->dispose();
		gRenderDevice->deleteBuffers(1, &copyTextureFBO);

		ubo_perFrame.release();

		{
			for (uint8 i = 0; i < (uint8)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
				if (pack_passes[i]) {
					delete pack_passes[i];
					pack_passes[i] = nullptr;
				}
			}
			directLightingPass->destroyResources(cmdList);
			indirectLightingPass->destroyResources(cmdList);
			screenSpaceReflectionPass->destroyResources(cmdList);
			translucency_pass->releaseResources(cmdList);
		}

		{
			skyboxPass->destroyResources(cmdList);
			anselSkyPass->destroyResources(cmdList);
			skyAtmospherePass->destroyResources(cmdList);
			volumetricCloud->destroyResources(cmdList);
		}

		{
			depthPrepass->destroyResources(cmdList);
			sunShadowMap->destroyResources(cmdList);
			omniShadowPass->destroyResources(cmdList);
			visualizeBuffer->destroyResources(cmdList);
		}

		{
			godRay->releaseResources(cmdList);
			ssao->releaseResources(cmdList);
			bloomSetup->releaseResources(cmdList);
			bloomPass->releaseResources(cmdList);
			toneMapping->releaseResources(cmdList);
			fxaa->releaseResources(cmdList);
			depthOfField->releaseResources(cmdList);
		}

		cmdList.flushAllCommands();
	}

	DEFINE_GLOBAL_RENDER_ROUTINE(DeferredRenderer, DeferredRenderer::internal_initGlobalResources, DeferredRenderer::internal_destroyGlobalResources);

}
