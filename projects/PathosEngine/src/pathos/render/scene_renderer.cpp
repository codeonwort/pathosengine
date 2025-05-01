#include "scene_renderer.h"

// Engine core
#include "pathos/engine_policy.h"
#include "pathos/console.h"
#include "pathos/util/log.h"
#include "pathos/util/cpu_profiler.h"

// Render core
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/gl_debug_group.h"
#include "pathos/rhi/texture.h"
#include "pathos/render/render_target.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/material/material_shader.h"

// Render passes
#include "pathos/render/depth_prepass.h"
#include "pathos/render/gbuffer_pass.h"
#include "pathos/render/shadow_directional.h"
#include "pathos/render/shadow_omni.h"
#include "pathos/render/skybox.h"
#include "pathos/render/sky_panorama.h"
#include "pathos/render/sky_atmosphere.h"
#include "pathos/render/volumetric_clouds.h"
#include "pathos/render/god_ray.h"
#include "pathos/render/visualize_buffer.h"
#include "pathos/render/visualize_light_probe.h"
#include "pathos/render/visualize_sky_occlusion.h"
#include "pathos/render/auto_exposure.h"
#include "pathos/render/forward/translucency_rendering.h"
#include "pathos/render/direct_lighting.h"
#include "pathos/render/indirect_lighting.h"
#include "pathos/render/resolve_unlit.h"
#include "pathos/render/screen_space_reflection.h"
#include "pathos/render/landscape_rendering.h"
#include "pathos/render/light_probe_baker.h"
#include "pathos/render/postprocessing/ssao.h"
#include "pathos/render/postprocessing/bloom_setup.h"
#include "pathos/render/postprocessing/bloom.h"
#include "pathos/render/postprocessing/tone_mapping.h"
#include "pathos/render/postprocessing/depth_of_field.h"
#include "pathos/render/postprocessing/anti_aliasing.h"
#include "pathos/render/postprocessing/anti_aliasing_fxaa.h"
#include "pathos/render/postprocessing/anti_aliasing_taa.h"
#include "pathos/render/postprocessing/super_res.h"
#include "pathos/render/postprocessing/super_res_fsr1.h"

// Render proxies
#include "pathos/scene/volumetric_cloud_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/mesh/static_mesh.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"
#include "badger/math/random.h"

namespace pathos {

	template<SceneRenderer::ECopyTextureMode copyMode>
	class CopyTextureFS : public ShaderStage {
	public:
		CopyTextureFS() : ShaderStage(GL_FRAGMENT_SHADER, "CopyTextureFS") {
			addDefine("COPY_MODE", (uint32)copyMode);
			setFilepath("copy_texture.fs.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_CopyTexture_Color, FullscreenVS, CopyTextureFS<SceneRenderer::ECopyTextureMode::CopyColor>);
	DEFINE_SHADER_PROGRAM2(Program_CopyTexture_LightProbeDepth, FullscreenVS, CopyTextureFS<SceneRenderer::ECopyTextureMode::LightProbeDepth>);

}

namespace pathos {

	static ConsoleVariable<int32> cvar_frustum_culling("r.frustum_culling", 1, "0 = disable, 1 = enable");
	static ConsoleVariable<int32> cvar_depth_prepass("r.depth_prepass", 1, "0 = disable, 1 = enable");
	static ConsoleVariable<int32> cvar_enable_ssr("r.ssr.enable", 1, "0 = disable SSR, 1 = enable SSR");
	static ConsoleVariable<int32> cvar_enable_bloom("r.bloom", 1, "0 = disable bloom, 1 = enable bloom");
	static ConsoleVariable<int32> cvar_enable_dof("r.dof.enable", 1, "0 = disable DoF, 1 = enable DoF");

	static ConsoleVariable<int32> cvar_exposure_mode("r.exposure.mode", 2, "0 = manual exposure, 1 = auto exposure by average luminance, 2 = auto exposure by luminance histogram. If manual, exposure value is set by r.exposure.override");
	static ConsoleVariable<float> cvar_exposure_override("r.exposure.override", 0.0f, "Exposure value for manual exposure mode");
	static ConsoleVariable<float> cvar_exposure_compensation("r.exposure.compensation", 0.0f, "Exposure bias applied after either auto or manual exposure is determined");

	SceneRenderer::SceneRenderer()
		: scene(nullptr)
		, camera(nullptr)
	{
		// #todo-renderer: Restore forward shading pipeline.
		// ...

		// [GDC2016] INSIDE uses 16 samples from Halton(2,3).
		badger::HaltonSequence(2, JITTER_SEQ_LENGTH, temporalJitterSequenceX);
		badger::HaltonSequence(3, JITTER_SEQ_LENGTH, temporalJitterSequenceY);
		for (uint32 i = 0; i < JITTER_SEQ_LENGTH; ++i) {
			temporalJitterSequenceX[i] = temporalJitterSequenceX[i] - 0.5f;
			temporalJitterSequenceY[i] = temporalJitterSequenceY[i] - 0.5f;
		}
	}

	SceneRenderer::~SceneRenderer() {
		CHECK(destroyed);
	}

	void SceneRenderer::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fboScreenshot);
	}

	void SceneRenderer::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fboScreenshot);
		}
		destroyed = true;
	}

	void SceneRenderer::setSceneRenderSettings(const SceneRenderSettings& settings) {
		CHECK(settings.isValid());

		sceneRenderSettings = settings;
		frameCounter = sceneRenderSettings.frameCounter; // #todo-renderer: Duplicate with SceneProxy::frameNumber
		
		if (settings.finalRenderTarget != nullptr) {
			setFinalRenderTarget(settings.finalRenderTarget);
		}
	}

	void SceneRenderer::setFinalRenderTarget(RenderTargetView* inFinalRenderTarget) {
		CHECKF(inFinalRenderTarget != nullptr, "null is not accepted. Use setFinalRenderTargetToBackbuffer() for backbuffer");
		CHECKF(inFinalRenderTarget->isDepthFormat() == false, "Depth format is not supported yet");
		finalRenderTarget = inFinalRenderTarget;
	}

	void SceneRenderer::setFinalRenderTargetToBackbuffer() {
		finalRenderTarget = 0;
	}

	void SceneRenderer::renderScene(
		RenderCommandList& cmdList,
		SceneRenderTargets* inSceneRenderTargets,
		SceneProxy* inScene,
		Camera* inCamera)
	{
		sceneRenderTargets = inSceneRenderTargets;
		scene = inScene;
		camera = inCamera;

		SCOPED_GPU_COUNTER(SceneRenderer);
		CHECK(sceneRenderSettings.isValid());

		const bool bEnableResolutionScaling = (scene->sceneProxySource == SceneProxySource::MainScene);
		const bool bLightProbeRendering = isLightProbeRendering(scene->sceneProxySource);

		const EAutoExposureMode autoExposureMode = (EAutoExposureMode)badger::clamp(0, cvar_exposure_mode.getInt(), 2);
		
		// Renderer-level conditions. Each render pass might reject to execute inside its logic.
		const bool bRenderDepthPrepass              = (cvar_depth_prepass.getInt() != 0);
		const bool bRenderGodRay                    = (bLightProbeRendering == false);
		const bool bRenderVolumetricCloud           = (bLightProbeRendering == false);
		// #todo-light-probe: Render sky for light probes?
		const bool bRenderSky                       = true || (bLightProbeRendering == false);
		const bool bRenderIndirectLighting          = (bLightProbeRendering == false && cvar_indirectLighting.getInt() != 0);
		const bool bRenderSSR                       = (bLightProbeRendering == false && cvar_enable_ssr.getInt() != 0);
		const bool bRenderAutoExposure              = (bLightProbeRendering == false && autoExposureMode != EAutoExposureMode::Manual);
		const bool bRenderLightProbeVisualization   = (bLightProbeRendering == false);
		const bool bRenderBufferVisualization       = (bLightProbeRendering == false);
		const bool bRenderSkyOcclusionVisualization = (bLightProbeRendering == false);

		cmdList.sceneProxy = inScene;
		cmdList.sceneRenderTargets = sceneRenderTargets;
		sceneRenderTargets->reallocSceneTextures(
			cmdList,
			scene->sceneProxySource,
			sceneRenderSettings.sceneWidth,
			sceneRenderSettings.sceneHeight,
			bEnableResolutionScaling);

		// Prepare fallback material.
		if (fallbackMaterial.get() == nullptr) {
			fallbackMaterial = uniquePtr<Material>(Material::createMaterialInstance("solid_color"));
			fallbackMaterial->setConstantParameter("albedo", vector3(0.5f, 0.5f, 0.5f));
			fallbackMaterial->setConstantParameter("metallic", 0.0f);
			fallbackMaterial->setConstantParameter("roughness", 0.0f);
			fallbackMaterial->setConstantParameter("emissive", vector3(0.0f));
		}
		if (indirectDrawDummyMaterial.get() == nullptr) {
			indirectDrawDummyMaterial = uniquePtr<Material>(Material::createMaterialInstance("indirect_draw_dummy"));
		}

		// #todo-multiview
		{
			SCOPED_CPU_COUNTER(UpdateUniformBuffer);

			// These should be updated before updateSceneUniformBuffer
			scene->createViewDependentRenderProxy(camera->getViewMatrix());

			// Update ubo_perFrame
			updateSceneUniformBuffer(cmdList, scene, camera);
		}

		{
			SCOPED_CPU_COUNTER(PreprocessLandscape);
			landscapeRendering->preprocess(cmdList, scene, camera);
		}

		if (cvar_frustum_culling.getInt() != 0) {
			scene->checkFrustumCulling(*camera);
		}

		if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
			cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		}
		if (bLightProbeRendering) {
			cmdList.disable(GL_CULL_FACE);
		} else {
			cmdList.enable(GL_CULL_FACE);
		}

		if (bRenderDepthPrepass) {
			SCOPED_CPU_COUNTER(RenderPreDepth);
			SCOPED_GPU_COUNTER(RenderPreDepth);
			depthPrepass->renderPreDepth(cmdList, scene, indirectDrawDummyMaterial.get(), landscapeRendering.get());
		}

		// #todo-light-probe: Don't need to render this per scene proxy,
		// but scene proxies for light probes are processed prior to the scene proxy for the main view.
		static auto cvarShadow = ConsoleVariableManager::get().find("r.shadow");
		CHECKF(cvarShadow != nullptr, "CVar is missing: r.shadow");
		if (cvarShadow->getInt() != 0) {
			SCOPED_CPU_COUNTER(RenderCascadedShadowMap);
			SCOPED_GPU_COUNTER(RenderCascadedShadowMap);
			// #todo-performance: This is incredibly slow in debug build
			sunShadowMap->renderShadowMap(cmdList, scene, camera, indirectDrawDummyMaterial.get(), cachedPerFrameUBOData);
		}

		// #todo-light-probe: Don't need to render this per scene proxy,
		// but scene proxies for light probes are processed prior to the scene proxy for the main view.
		{
			SCOPED_CPU_COUNTER(RenderOmniShadowMaps);
			SCOPED_GPU_COUNTER(RenderOmniShadowMaps);
			// #todo-performance: This is incredibly super slow in debug build
			omniShadowPass->renderShadowMaps(cmdList, scene, camera);
		}

		if (bRenderVolumetricCloud) {
			SCOPED_CPU_COUNTER(VolumetricCloud);
			SCOPED_GPU_COUNTER(VolumetricCloud);
			volumetricCloud->renderVolumetricCloud(cmdList, scene);
		}

		{
			SCOPED_CPU_COUNTER(BasePass);
			SCOPED_GPU_COUNTER(BasePass);
			gbufferPass->renderGBuffers(cmdList, scene, bRenderDepthPrepass, landscapeRendering.get());
		}

		if (bRenderGodRay) {
			SCOPED_CPU_COUNTER(GodRay);
			SCOPED_GPU_COUNTER(GodRay);
			godRay->renderGodRay(cmdList, scene, camera, this);
		}

		{
			SCOPED_CPU_COUNTER(SSAO);
			SCOPED_GPU_COUNTER(SSAO);
			ssao->renderAmbientOcclusion(cmdList);
		}

		{
			SCOPED_CPU_COUNTER(ClearSceneColor);
			SCOPED_GPU_COUNTER(ClearSceneColor);
			SCOPED_DRAW_EVENT(ClearSceneColor);
			GLfloat* clearValues = (GLfloat*)cmdList.allocateSingleFrameMemory(4 * sizeof(GLfloat));
			for (int32 i = 0; i < 4; ++i) clearValues[i] = 0.0f;
			cmdList.clearTexImage(
				sceneRenderTargets->sceneColor,
				0, // mip
				GL_RGBA,
				GL_FLOAT,
				clearValues);
		}

		{
			SCOPED_CPU_COUNTER(DirectLighting);
			SCOPED_GPU_COUNTER(DirectLighting);

			directLightingPass->bindFramebuffer(cmdList);
			directLightingPass->renderDirectLighting(cmdList, scene, camera);
		}

		{
			SCOPED_CPU_COUNTER(SkyLighting);
			SCOPED_GPU_COUNTER(SkyLighting);
			SCOPED_DRAW_EVENT(SkyLighting);

			// Sometimes sky lighting is invalidated due to, for instance, world transition.
			// Clear current sky lighting textures and let sky passes update them.
			if (scene->bInvalidateSkyLighting) {
				SCOPED_GPU_COUNTER(ClearSkyLighting);
				SCOPED_DRAW_EVENT(ClearSkyLighting);

				if (sceneRenderTargets->skyDiffuseSH != nullptr) {
					vector4 clearBuf[9];
					for (int32 i = 0; i < 9; ++i) clearBuf[i] = vector4(0.0f);
					sceneRenderTargets->skyDiffuseSH->writeToGPU_renderThread(cmdList, 0, sizeof(clearBuf), clearBuf);
				}

				float clearValues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				if (sceneRenderTargets->skyPrefilteredMap != 0) {
					pathos::clearTextureCube(
						cmdList,
						sceneRenderTargets->skyPrefilteredMap,
						sceneRenderTargets->skyPrefilterMapSize,
						0, // start mip level
						pathos::SKY_PREFILTER_MAP_MIP_COUNT, // mip count to clear
						EClearTextureFormat::RGBA16f,
						clearValues);
				}
			}

			const bool bRenderSkybox = scene->isSkyboxValid();
			const bool bRenderPanorama = scene->isPanoramaSkyValid();
			const bool bRenderAtmosphere = scene->isSkyAtmosphereValid();
			{
				// #todo-sky: What to choose when multiple sky proxies are active?
				//int32 numActiveSkies = (int32)bRenderSkybox + (int32)bRenderPanorama + (int32)bRenderAtmosphere;
				//CHECKF(numActiveSkies <= 1, "At most one sky representation is allowed at the same time");
			}
			if (bRenderSky) {
				if (scene->isSkyboxValid()) {
					skyboxPass->renderSkybox(cmdList, scene);
				} else if (scene->isPanoramaSkyValid()) {
					panoramaSkyPass->renderPanoramaSky(cmdList, scene);
				} else if (scene->isSkyAtmosphereValid()) {
					skyAtmospherePass->renderSkyAtmosphere(cmdList, scene, camera, fullscreenQuad);
				}
			}
		}

		if (bRenderIndirectLighting) {
			SCOPED_CPU_COUNTER(IndirectLighting);
			SCOPED_GPU_COUNTER(IndirectLighting);

			indirectLightingPass->renderIndirectLighting(cmdList, scene, camera, fullscreenQuad);
		}

		// Add unlit and emissive
		{
			SCOPED_CPU_COUNTER(ResolveUnlit);
			SCOPED_GPU_COUNTER(ResolveUnlit);
			resolveUnlitPass->renderUnlit(cmdList, fullscreenQuad);
		}

		if (bRenderSSR) {
			SCOPED_CPU_COUNTER(ScreenSpaceReflection);
			SCOPED_GPU_COUNTER(ScreenSpaceReflection);
			screenSpaceReflectionPass->renderScreenSpaceReflection(cmdList, scene, camera, fullscreenQuad);
		}

		if (bRenderVolumetricCloud) {
			SCOPED_CPU_COUNTER(VolumetricCloudPost);
			SCOPED_GPU_COUNTER(VolumetricCloudPost);
			volumetricCloud->renderVolumetricCloudPost(cmdList, scene);
		}

		{
			SCOPED_CPU_COUNTER(Translucency);
			SCOPED_GPU_COUNTER(Translucency);
			translucency_pass->renderTranslucency(cmdList, scene, camera);
		}

		if (bRenderGodRay) {
			SCOPED_CPU_COUNTER(GodRayPost);
			SCOPED_GPU_COUNTER(GodRayPost);
			godRay->renderGodRayPost(cmdList, scene);
		}

		if (bRenderAutoExposure) {
			SCOPED_CPU_COUNTER(AutoExposure);
			SCOPED_GPU_COUNTER(AutoExposure);
			autoExposurePass->renderAutoExposure(cmdList, scene, autoExposureMode);
		}

		if (bRenderLightProbeVisualization) {
			SCOPED_CPU_COUNTER(VisualizeLightProbe);
			SCOPED_GPU_COUNTER(VisualizeLightProbe);
			visualizeLightProbe->render(cmdList, scene, camera);
		}

		if (bLightProbeRendering && scene->lightProbeShIndex != IrradianceProbeAtlasDesc::INVALID_TILE_ID) {
			LightProbeBaker::get().bakeLightProbeSH_renderThread(
				cmdList, scene->lightProbeColorCubemap, scene->lightProbeDepthCubemap,
				scene->irradianceSHBuffer, scene->lightProbeShIndex);

			LightProbeBaker::get().bakeOctahedralDepthAtlas_renderThread(
				cmdList, scene->lightProbeDepthCubemap, scene->depthProbeAtlas, scene->lightProbeDepthAtlasCoordAndSize);
		}

		//////////////////////////////////////////////////////////////////////////
		// Post-processing
		// 
		// #note: Post processing passes are not supposed to be enabled in light probe rendering.

		GLuint sceneAfterLastPP = 0;

		if (sceneRenderSettings.enablePostProcess == false)
		{
			SCOPED_DRAW_EVENT(BlitToFinalTarget);
			SCOPED_GPU_COUNTER(BlitToFinalTarget);

			copyTexture(cmdList, sceneRenderTargets->sceneColor, getFinalRenderTarget(),
				sceneRenderTargets->unscaledSceneWidth, sceneRenderTargets->unscaledSceneHeight);
			
			if (bLightProbeRendering && sceneRenderSettings.finalDepthTarget != nullptr) {
				GLuint depthTarget = sceneRenderSettings.finalDepthTarget->getGLName();
				copyTexture(cmdList, sceneRenderTargets->sceneDepth, depthTarget,
					sceneRenderTargets->unscaledSceneWidth, sceneRenderTargets->unscaledSceneHeight,
					ECopyTextureMode::LightProbeDepth);
			}
		}
		else
		{
			SCOPED_CPU_COUNTER(PostProcessing);
			SCOPED_GPU_COUNTER(PostProcessing);

			const EAntiAliasingMethod antiAliasing = getAntiAliasingMethod();
			const ESuperResolutionMethod superResMethod = pathos::getSuperResolutionMethod();

			enum class EPostProcessOrder : uint8 {
				Bloom,
				ToneMapping,
				AntiAliasing,
				SuperResolution,
				DepthOfField,
				MAX
			};
			constexpr uint8 NumPPOrders = (uint8)EPostProcessOrder::MAX;
			bool postProcessEnableFlags[NumPPOrders];
			auto setEnablePP = [&postProcessEnableFlags](EPostProcessOrder pp, bool bEnabled) {
				postProcessEnableFlags[(uint8)pp] = bEnabled;
			};
			auto isPPEnabled = [&postProcessEnableFlags](EPostProcessOrder pp) {
				return postProcessEnableFlags[(uint8)pp];
			};
			// Assumes isPPEnabled(pp) == true
			auto isPPFinal = [&postProcessEnableFlags, NumPPOrders](EPostProcessOrder pp) {
				for (uint8 i = (uint8)pp + 1; i < NumPPOrders; ++i) {
					if (postProcessEnableFlags[i]) return false;
				}
				return true;
			};
			setEnablePP(EPostProcessOrder::Bloom, cvar_enable_bloom.getInt() != 0);
			setEnablePP(EPostProcessOrder::ToneMapping, true);
			setEnablePP(EPostProcessOrder::AntiAliasing, antiAliasing != EAntiAliasingMethod::NoAA);
			setEnablePP(EPostProcessOrder::SuperResolution, superResMethod != ESuperResolutionMethod::Disabled);
			setEnablePP(EPostProcessOrder::DepthOfField, (cvar_enable_dof.getInt() != 0) && depthOfField->isAvailable());

			sceneAfterLastPP = sceneRenderTargets->sceneColor;

			fullscreenQuad->bindFullAttributesVAO(cmdList);

			// Make half res version of sceneColor. A common source for PPs that are too expensive to run in full res.
			const bool bNeedsHalfResSceneColor = isPPEnabled(EPostProcessOrder::Bloom); // NOTE: Add other conditions if needed.
			if (bNeedsHalfResSceneColor) {
				SCOPED_CPU_COUNTER(SceneColorDownsample);
				SCOPED_DRAW_EVENT(SceneColorDownsample);
				SCOPED_GPU_COUNTER(SceneColorDownsample);
				copyTexture(cmdList, sceneRenderTargets->sceneColor, sceneRenderTargets->sceneColorHalfRes,
					sceneRenderTargets->sceneWidth / 2, sceneRenderTargets->sceneHeight / 2);
			}

			// Post Process: Bloom
			if (isPPEnabled(EPostProcessOrder::Bloom)) {
				SCOPED_CPU_COUNTER(Bloom);
				SCOPED_GPU_COUNTER(Bloom);

				bloomSetup->setInput(EPostProcessInput::PPI_0, sceneRenderTargets->sceneColorHalfRes);
				bloomSetup->setOutput(EPostProcessOutput::PPO_0, sceneRenderTargets->sceneBloomSetup);
				bloomSetup->renderPostProcess(cmdList, fullscreenQuad);

				bloomPass->setInput(EPostProcessInput::PPI_0, sceneRenderTargets->sceneBloomSetup);
				bloomPass->renderPostProcess(cmdList, fullscreenQuad);
			}

			// Post Process: Tone Mapping
			{
				SCOPED_CPU_COUNTER(ToneMapping);
				SCOPED_GPU_COUNTER(ToneMapping);

				const bool isFinalPP = isPPFinal(EPostProcessOrder::ToneMapping);
				const float exposureOverride = cvar_exposure_override.getFloat();
				const float exposureCompensation = cvar_exposure_compensation.getFloat();
				const bool bApplyBloom = isPPEnabled(EPostProcessOrder::Bloom);
				GLuint black2D = gEngine->getSystemTexture2DBlack()->internal_getGLName();

				GLuint bloom = bApplyBloom ? sceneRenderTargets->sceneBloomChain : black2D;
				GLuint toneMappingRenderTarget = isFinalPP ? sceneRenderTargets->sceneFinal : sceneRenderTargets->sceneColorToneMapped;

				GLuint luminanceTexture; uint32 luminanceTargetMip; bool bLuminanceLogScale;
				autoExposurePass->getAutoExposureResults(*sceneRenderTargets, autoExposureMode, luminanceTexture, luminanceTargetMip, bLuminanceLogScale);
				
				toneMapping->setParameters(
					bRenderAutoExposure, luminanceTargetMip, bLuminanceLogScale,
					exposureOverride, exposureCompensation,
					bApplyBloom);

				// #todo-postprocess: Don't mix bloom inside of tone mapping shader.
				toneMapping->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
				toneMapping->setInput(EPostProcessInput::PPI_1, bloom);
				toneMapping->setInput(EPostProcessInput::PPI_2, luminanceTexture);
				toneMapping->setOutput(EPostProcessOutput::PPO_0, toneMappingRenderTarget);
				toneMapping->renderPostProcess(cmdList, fullscreenQuad);

				sceneAfterLastPP = toneMappingRenderTarget;
			}

			// Post Process: Anti-aliasing
			switch (antiAliasing) {
				case EAntiAliasingMethod::NoAA:
				{
					// Do nothing
					break;
				}
				case EAntiAliasingMethod::FXAA:
				{
					SCOPED_CPU_COUNTER(FXAA);
					SCOPED_GPU_COUNTER(FXAA);

					const bool isFinalPP = isPPFinal(EPostProcessOrder::AntiAliasing);
					const GLuint aaRenderTarget = isFinalPP ? sceneRenderTargets->sceneFinal : sceneRenderTargets->sceneColorAA;

					fxaa->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
					fxaa->setOutput(EPostProcessOutput::PPO_0, aaRenderTarget);
					fxaa->renderPostProcess(cmdList, fullscreenQuad);

					sceneAfterLastPP = aaRenderTarget;
					break;
				}
				case EAntiAliasingMethod::TAA:
				{
					SCOPED_CPU_COUNTER(TAA);
					SCOPED_GPU_COUNTER(TAA);

					const bool isFinalPP = isPPFinal(EPostProcessOrder::AntiAliasing);
					const GLuint aaRenderTarget = isFinalPP ? sceneRenderTargets->sceneFinal : sceneRenderTargets->sceneColorAA;

					taa->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
					taa->setInput(EPostProcessInput::PPI_1, sceneRenderTargets->sceneColorHistory);
					taa->setInput(EPostProcessInput::PPI_2, sceneRenderTargets->sceneDepth);
					taa->setInput(EPostProcessInput::PPI_3, sceneRenderTargets->velocityMap);
					taa->setInput(EPostProcessInput::PPI_4, sceneRenderTargets->gbufferA);
					taa->setOutput(EPostProcessOutput::PPO_0, aaRenderTarget);
					taa->renderPostProcess(cmdList, fullscreenQuad);

					{
						SCOPED_DRAW_EVENT(SaveSceneColorHistory);
						copyTexture(cmdList,
							aaRenderTarget, sceneRenderTargets->sceneColorHistory,
							sceneRenderTargets->sceneWidth, sceneRenderTargets->sceneHeight);
					}

					sceneAfterLastPP = aaRenderTarget;
					break;
				}
				default:
				{
					CHECK_NO_ENTRY();
					break;
				}
			}

			// Super resolution
			// NOTE: Should run after tone mapping and before any noise-introducing PPs.
			if (isPPEnabled(EPostProcessOrder::SuperResolution)) {
				SCOPED_CPU_COUNTER(SuperResolution);
				SCOPED_GPU_COUNTER(SuperResolution);

				switch (superResMethod) {
				case ESuperResolutionMethod::FSR1:
					{
						const bool isFinalPP = isPPFinal(EPostProcessOrder::SuperResolution);
						//const GLuint renderTarget = isFinalPP ? sceneRenderTargets->sceneFinal : sceneRenderTargets->sceneColorUpscaled;
						const GLuint renderTarget = sceneRenderTargets->sceneColorUpscaled;

						fsr1->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
						fsr1->setOutput(EPostProcessOutput::PPO_0, renderTarget);
						fsr1->renderPostProcess(cmdList, fullscreenQuad);

						sceneAfterLastPP = renderTarget;

						// #todo-fsr1: glReadPixels() produces all zero when FSR1 is enabled.
						// sceneFinal is correct in RenderDoc, No glGetError(), 
						// Even memory barrier with GL_ALL_BARRIER_BITS does not solve the problem.
						// -> Workaround: FSR1 always renders to sceneColorUpscaled, copy sceneColorUpscaled to sceneFinal.
						if (isFinalPP) {
							copyTexture(cmdList, sceneAfterLastPP, sceneRenderTargets->sceneFinal,
								sceneRenderTargets->sceneWidthSuperRes, sceneRenderTargets->sceneHeightSuperRes);
							sceneAfterLastPP = sceneRenderTargets->sceneFinal;
						}
					}
					break;

				default:
					break;
					
				}
			}

			// Post Process: Depth of Field
			if (isPPEnabled(EPostProcessOrder::DepthOfField)) {
				SCOPED_CPU_COUNTER(DepthOfField);
				SCOPED_GPU_COUNTER(DepthOfField);

				const GLuint dofInput = sceneRenderTargets->sceneColorDoFInput;
				const GLuint dofRenderTarget = sceneRenderTargets->sceneFinal;

				// Force rgba32f input.
				// #todo-postprocess: Don't copy and use sceneAfterLastPP directly if it's rgba32f.
				copyTexture(cmdList, sceneAfterLastPP, dofInput,
					sceneRenderTargets->sceneWidthSuperRes, sceneRenderTargets->sceneHeightSuperRes);

				depthOfField->setInput(EPostProcessInput::PPI_0, dofInput);
				depthOfField->setOutput(EPostProcessOutput::PPO_0, dofRenderTarget);
				depthOfField->renderPostProcess(cmdList, fullscreenQuad);

				sceneAfterLastPP = dofRenderTarget;
			}

		}

		// Assumes rendering result is always written to sceneFinal and it's pixel format is rgba16f.
		if (scene->bScreenshotReserved) {
			SCOPED_CPU_COUNTER(ReadbackScreenshot);
			SCOPED_DRAW_EVENT(ReadbackScreenshot);

			CHECK(sceneAfterLastPP == sceneRenderTargets->sceneFinal);

			cmdList.bindFramebuffer(GL_READ_FRAMEBUFFER, fboScreenshot);
			cmdList.namedFramebufferTexture(fboScreenshot, GL_COLOR_ATTACHMENT0, sceneRenderTargets->sceneFinal, 0);
			cmdList.namedFramebufferReadBuffer(fboScreenshot, GL_COLOR_ATTACHMENT0);

			vector2i screenshotSize((int32)sceneRenderTargets->sceneWidthSuperRes, (int32)sceneRenderTargets->sceneHeightSuperRes);

			scene->screenshotSize = screenshotSize;
			scene->screenshotRawData.resize(4 * screenshotSize.x * screenshotSize.y);
			cmdList.pixelStorei(GL_PACK_ALIGNMENT, 1);
			cmdList.readPixels(0, 0, screenshotSize.x, screenshotSize.y, GL_RGBA, GL_HALF_FLOAT, scene->screenshotRawData.data());
			cmdList.pixelStorei(GL_PACK_ALIGNMENT, 4);
		}

		if (sceneAfterLastPP != 0) {
			SCOPED_CPU_COUNTER(BlitToFinalTarget);
			SCOPED_GPU_COUNTER(BlitToFinalTarget);
			SCOPED_DRAW_EVENT(BlitToFinalTarget);
			copyTexture(cmdList, sceneAfterLastPP, getFinalRenderTarget(),
				sceneRenderTargets->unscaledSceneWidth, sceneRenderTargets->unscaledSceneHeight);
		}

		// Debug pass (r.viewmode)
		if (bRenderBufferVisualization) {
			SCOPED_CPU_COUNTER(VisualizeBuffer);
			SCOPED_GPU_COUNTER(VisualizeBuffer);
			visualizeBuffer->render(cmdList, scene, camera);
		}

		if (bRenderSkyOcclusionVisualization) {
			SCOPED_CPU_COUNTER(VisualizeSkyOcclusion);
			SCOPED_GPU_COUNTER(VisualizeSkyOcclusion);
			visualizeSkyOcclusionPass->renderSkyOcclusion(cmdList, scene, camera);
		}

		sceneRenderTargets = nullptr;
		scene = nullptr;
		camera = nullptr;
	}

	void SceneRenderer::copyTexture(
		RenderCommandList& cmdList, GLuint source,
		GLuint target, uint32 targetWidth, uint32 targetHeight,
		ECopyTextureMode copyMode)
	{
		GLuint program = 0;
		switch (copyMode) {
			case ECopyTextureMode::CopyColor:
				program = FIND_SHADER_PROGRAM(Program_CopyTexture_Color).getGLName();
				break;
			case ECopyTextureMode::LightProbeDepth:
				program = FIND_SHADER_PROGRAM(Program_CopyTexture_LightProbeDepth).getGLName();
				break;
			default:
				CHECK_NO_ENTRY();
		}

		cmdList.useProgram(program);
		if (target == 0) {
			cmdList.bindFramebuffer(GL_FRAMEBUFFER, 0);
		} else {
			cmdList.bindFramebuffer(GL_FRAMEBUFFER, SceneRenderer::copyTextureFBO);
			cmdList.namedFramebufferTexture(SceneRenderer::copyTextureFBO, GL_COLOR_ATTACHMENT0, target, 0);
		}
		cmdList.viewport(0, 0, targetWidth, targetHeight);
		cmdList.bindTextureUnit(0, source);
		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
		fullscreenQuad->unbindVAO(cmdList);
	}

	GLuint SceneRenderer::getFinalRenderTarget() const {
		if (finalRenderTarget == nullptr) {
			return 0; // Default backbuffer
		}
		return finalRenderTarget->getGLName();
	}

	void SceneRenderer::updateSceneUniformBuffer(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) {
		const matrix4& projMatrix = camera->getProjectionMatrix();
		const bool bMainScene = (scene->sceneProxySource == SceneProxySource::MainScene);
		const bool bSunExists = (scene->proxyList_directionalLight.size() > 0);
		const DirectionalLightProxy sunProxy = bSunExists ? *(scene->proxyList_directionalLight[0]) : DirectionalLightProxy::createDummy();

		auto buildResolution = [](uint32 w, uint32 h) -> vector4 {
			return vector4((float)w, (float)h, 1.0f / (float)w, 1.0f / (float)h);
		};

		// ----------------------------------------------------------------

		UBO_PerFrame data;

		data.view         = camera->getViewMatrix();
		data.inverseView  = glm::inverse(data.view);
		data.view3x3      = matrix3x4(data.view);
		data.viewProj     = camera->getViewProjectionMatrix();
		data.proj         = projMatrix;
		data.inverseProj  = glm::inverse(projMatrix);

		if (frameCounter != 0) {
			data.prevView        = prevView;
			data.prevInverseView = prevInverseView;
			data.prevViewProj    = prevViewProj;
		} else {
			data.prevView        = data.view;
			data.prevInverseView = data.inverseView;
			data.prevViewProj    = data.viewProj;
		}

		float tempJitterX = 0.0f, tempJitterY = 0.0f;
		if (bMainScene && getAntiAliasingMethod() == EAntiAliasingMethod::TAA) {
			uint32 jitterIx = (scene->frameNumber) % JITTER_SEQ_LENGTH;
			float K = getTemporalJitterMultiplier();
			// It reduces the effectiveness of TAA, but also relax jittering when super resolution is enabled.
			K /= getSuperResolutionScaleFactor();
			tempJitterX = K * temporalJitterSequenceX[jitterIx] / (float)sceneRenderTargets->sceneWidth;
			tempJitterY = K * temporalJitterSequenceY[jitterIx] / (float)sceneRenderTargets->sceneHeight;
		}
		data.temporalJitter    = vector4(tempJitterX, tempJitterY, 0.0f, 0.0f);
		data.screenResolution  = buildResolution(sceneRenderTargets->sceneWidth, sceneRenderTargets->sceneHeight);
		data.projParams        = vector4(camera->getZNear(), camera->getZFar(), camera->getFovYRadians(), camera->getAspectRatio());
		data.time              = vector4(gEngine->getWorldTime(), scene->deltaSeconds, 0.0f, 0.0f);

		data.cameraDirectionVS = vector3(camera->getViewMatrix() * vector4(camera->getEyeVector(), 0.0f));
		data.bReverseZ         = (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) ? 1 : 0;

		data.cameraPositionVS  = vector3(camera->getViewMatrix() * vector4(camera->getPosition(), 1.0f));
		data.__pad0            = 0.0f;

		data.cameraPositionWS  = camera->getPosition();
		data.sunExists         = bSunExists; // Treat the first directional light as Sun.

		data.sunLight          = sunProxy;

		ubo_perFrame->update(cmdList, UBO_PerFrame::BINDING_POINT, &data);

		if (bMainScene) {
			prevView = data.view;
			prevInverseView = data.inverseView;
			prevViewProj = data.viewProj;
		}

		cachedPerFrameUBOData = std::move(data);
	}

}

namespace pathos {
	
	uniquePtr<Material>                  SceneRenderer::fallbackMaterial;
	uniquePtr<Material>                  SceneRenderer::indirectDrawDummyMaterial;
	MeshGeometry*                        SceneRenderer::fullscreenQuad;
	GLuint                               SceneRenderer::copyTextureFBO = 0;
	
	uniquePtr<UniformBuffer>             SceneRenderer::ubo_perFrame;

	uniquePtr<LandscapeRendering>        SceneRenderer::landscapeRendering;

	// G-buffer rendering
	uniquePtr<DepthPrepass>              SceneRenderer::depthPrepass;
	uniquePtr<GBufferPass>               SceneRenderer::gbufferPass;
	uniquePtr<ResolveUnlitPass>          SceneRenderer::resolveUnlitPass;

	// Shadowmap rendering
	uniquePtr<DirectionalShadowMap>      SceneRenderer::sunShadowMap;
	uniquePtr<OmniShadowPass>            SceneRenderer::omniShadowPass;
	
	// Local & global illumination
	uniquePtr<DirectLightingPass>        SceneRenderer::directLightingPass;
	uniquePtr<IndirectLightingPass>      SceneRenderer::indirectLightingPass;
	uniquePtr<ScreenSpaceReflectionPass> SceneRenderer::screenSpaceReflectionPass;

	// Sky & atmosphere
	uniquePtr<SkyboxPass>                SceneRenderer::skyboxPass;
	uniquePtr<PanoramaSkyPass>           SceneRenderer::panoramaSkyPass;
	uniquePtr<SkyAtmospherePass>         SceneRenderer::skyAtmospherePass;
	uniquePtr<VolumetricCloudPass>       SceneRenderer::volumetricCloud;

	// Translucency
	uniquePtr<TranslucencyRendering>     SceneRenderer::translucency_pass;

	// Auto exposure
	uniquePtr<AutoExposurePass>          SceneRenderer::autoExposurePass;

	// Debug rendering
	uniquePtr<VisualizeBufferPass>       SceneRenderer::visualizeBuffer;
	uniquePtr<VisualizeLightProbePass>   SceneRenderer::visualizeLightProbe;
	uniquePtr<VisualizeSkyOcclusionPass> SceneRenderer::visualizeSkyOcclusionPass;

	// Post-processing
	uniquePtr<GodRay>                    SceneRenderer::godRay;
	uniquePtr<SSAO>                      SceneRenderer::ssao;
	uniquePtr<BloomSetup>                SceneRenderer::bloomSetup;
	uniquePtr<BloomPass>                 SceneRenderer::bloomPass;
	uniquePtr<ToneMapping>               SceneRenderer::toneMapping;
	uniquePtr<FXAA>                      SceneRenderer::fxaa;
	uniquePtr<TAA>                       SceneRenderer::taa;
	uniquePtr<FSR1>                      SceneRenderer::fsr1;
	uniquePtr<DepthOfField>              SceneRenderer::depthOfField;

	void SceneRenderer::internal_initGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		gRenderDevice->createFramebuffers(1, &copyTextureFBO);
		cmdList.namedFramebufferDrawBuffer(copyTextureFBO, GL_COLOR_ATTACHMENT0);

		ubo_perFrame = makeUnique<UniformBuffer>();
		ubo_perFrame->init<UBO_PerFrame>("UBO_PerFrame");

		const int32 uboMaxSize = gRenderDevice->getCapabilities().glMaxUniformBlockSize;
		if (sizeof(UBO_PerFrame) > uboMaxSize) {
			char msg[256];
			sprintf_s(msg, "%s: UBO_PerFrame is bigger than the device's limit (%d > %d)",
				__FUNCTION__, (int32)sizeof(UBO_PerFrame), uboMaxSize);
			CHECKF(false, msg);
		}

		{
			landscapeRendering = makeUnique<LandscapeRendering>();
			landscapeRendering->initializeResources(cmdList);
		}

		{
			directLightingPass = makeUnique<DirectLightingPass>();
			indirectLightingPass = makeUnique<IndirectLightingPass>();
			screenSpaceReflectionPass = makeUnique<ScreenSpaceReflectionPass>();
			translucency_pass = makeUnique<TranslucencyRendering>();

			directLightingPass->initializeResources(cmdList);
			indirectLightingPass->initializeResources(cmdList);
			screenSpaceReflectionPass->initializeResources(cmdList);
			translucency_pass->initializeResources(cmdList);
		}

		{
			autoExposurePass = makeUnique<AutoExposurePass>();
			autoExposurePass->initializeResources(cmdList);
		}

		{
			depthPrepass = makeUnique<DepthPrepass>();
			depthPrepass->initializeResources(cmdList);

			gbufferPass = makeUnique<GBufferPass>();
			gbufferPass->initializeResources(cmdList);

			resolveUnlitPass = makeUnique<ResolveUnlitPass>();
			resolveUnlitPass->initializeResources(cmdList);
		}

		{
			skyboxPass = makeUnique<SkyboxPass>();
			skyboxPass->initializeResources(cmdList);

			panoramaSkyPass = makeUnique<PanoramaSkyPass>();
			panoramaSkyPass->initializeResources(cmdList);

			skyAtmospherePass = makeUnique<SkyAtmospherePass>();
			skyAtmospherePass->initializeResources(cmdList);

			volumetricCloud = makeUnique<VolumetricCloudPass>();
			volumetricCloud->initializeResources(cmdList);
		}

		{
			sunShadowMap = makeUnique<DirectionalShadowMap>();
			omniShadowPass = makeUnique<OmniShadowPass>();
			
			sunShadowMap->initializeResources(cmdList);
			omniShadowPass->initializeResources(cmdList);
		}

		{
			visualizeBuffer = makeUnique<VisualizeBufferPass>();
			visualizeBuffer->initializeResources(cmdList);

			visualizeLightProbe = makeUnique<VisualizeLightProbePass>();
			visualizeLightProbe->initializeResources(cmdList);

			visualizeSkyOcclusionPass = makeUnique<VisualizeSkyOcclusionPass>();
			visualizeSkyOcclusionPass->initializeResources(cmdList);
		}

		{
			godRay = makeUnique<GodRay>();
			ssao = makeUnique<SSAO>();
			bloomSetup = makeUnique<BloomSetup>();
			bloomPass = makeUnique<BloomPass>();
			toneMapping = makeUnique<ToneMapping>();
			fxaa = makeUnique<FXAA>();
			taa = pathos::makeUnique<TAA>();
			fsr1 = pathos::makeUnique<FSR1>();
			depthOfField = makeUnique<DepthOfField>();

			godRay->initializeResources(cmdList);
			ssao->initializeResources(cmdList);
			bloomSetup->initializeResources(cmdList);
			bloomPass->initializeResources(cmdList);
			toneMapping->initializeResources(cmdList);
			fxaa->initializeResources(cmdList);
			taa->initializeResources(cmdList);
			fsr1->initializeResources(cmdList);
			depthOfField->initializeResources(cmdList);
		}
	}

	void SceneRenderer::internal_destroyGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		fallbackMaterial.reset();
		indirectDrawDummyMaterial.reset();
		gRenderDevice->deleteFramebuffers(1, &copyTextureFBO);

		ubo_perFrame.reset();

#define RELEASEPASS(pass) { pass->releaseResources(cmdList); pass.reset(); }

		RELEASEPASS(landscapeRendering);

		RELEASEPASS(depthPrepass);
		RELEASEPASS(gbufferPass);
		RELEASEPASS(resolveUnlitPass);

		RELEASEPASS(sunShadowMap);
		RELEASEPASS(omniShadowPass);

		RELEASEPASS(directLightingPass);
		RELEASEPASS(indirectLightingPass);
		RELEASEPASS(screenSpaceReflectionPass);

		RELEASEPASS(skyboxPass);
		RELEASEPASS(panoramaSkyPass);
		RELEASEPASS(skyAtmospherePass);
		RELEASEPASS(volumetricCloud);

		RELEASEPASS(translucency_pass);

		RELEASEPASS(autoExposurePass);

		RELEASEPASS(visualizeBuffer);
		RELEASEPASS(visualizeLightProbe);
		RELEASEPASS(visualizeSkyOcclusionPass);

		RELEASEPASS(godRay);
		RELEASEPASS(ssao);
		RELEASEPASS(bloomSetup);
		RELEASEPASS(bloomPass);
		RELEASEPASS(toneMapping);
		RELEASEPASS(fxaa);
		RELEASEPASS(taa);
		RELEASEPASS(fsr1);
		RELEASEPASS(depthOfField);

#undef RELEASEPASS

		cmdList.flushAllCommands();
	}

	DEFINE_GLOBAL_RENDER_ROUTINE(SceneRenderer, SceneRenderer::internal_initGlobalResources, SceneRenderer::internal_destroyGlobalResources);

}
