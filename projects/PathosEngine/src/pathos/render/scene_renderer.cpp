#include "scene_renderer.h"

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
#include "pathos/render/postprocessing/super_res.h"
#include "pathos/render/postprocessing/super_res_fsr1.h"

#include "pathos/light/directional_light_component.h"
#include "pathos/light/point_light_component.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/mesh/mesh.h"
#include "pathos/texture/volume_texture.h"
#include "pathos/shader/shader_program.h"
#include "pathos/shader/material_shader.h"

#include "pathos/console.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/gl_debug_group.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"

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
	static ConsoleVariable<int32> cvar_enable_probegi("r.probegi.enable", 1, "Toggle probe GI");
	static ConsoleVariable<int32> cvar_enable_ssr("r.ssr.enable", 1, "0 = disable SSR, 1 = enable SSR");
	static ConsoleVariable<int32> cvar_enable_bloom("r.bloom", 1, "0 = disable bloom, 1 = enable bloom");
	static ConsoleVariable<int32> cvar_enable_dof("r.dof.enable", 1, "0 = disable DoF, 1 = enable DoF");
	static ConsoleVariable<int32> cvar_anti_aliasing("r.antialiasing.method", 1, "0 = disable, 1 = FXAA");

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
		uint32                sunExists;

		DirectionalLightProxy sunLight;
	};
	static constexpr GLuint SCENE_UNIFORM_BINDING_INDEX = 0;

	SceneRenderer::SceneRenderer()
		: scene(nullptr)
		, camera(nullptr)
	{
		// #todo-forward-shading: Restore forward shading pipeline.
		// ...

		sceneRenderTargets.useGBuffer = true;
	}

	SceneRenderer::~SceneRenderer() {
		CHECK(destroyed);
	}

	void SceneRenderer::initializeResources(RenderCommandList& cmdList) {
		sceneRenderTargets.reallocSceneTextures(cmdList, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight, false);
		cmdList.flushAllCommands();
		cmdList.sceneRenderTargets = &sceneRenderTargets;

		uboPerObject.init<Material::UBO_PerObject>("UBO_PerObject_BasePass");

		gRenderDevice->createFramebuffers(1, &fboScreenshot);
	}

	void SceneRenderer::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			destroySceneRenderTargets(cmdList);
			gRenderDevice->deleteFramebuffers(1, &fboScreenshot);
		}
		destroyed = true;
	}

	void SceneRenderer::setSceneRenderSettings(const SceneRenderSettings& settings) {
		CHECK(settings.isValid());

		sceneRenderSettings = settings;
		frameCounter = sceneRenderSettings.frameCounter; // #todo: Duplicate with SceneProxy::frameNumber
		
		if (settings.finalRenderTarget != nullptr) {
			setFinalRenderTarget(settings.finalRenderTarget);
		}
	}

	void SceneRenderer::setFinalRenderTarget(RenderTarget2D* inFinalRenderTarget) {
		CHECKF(inFinalRenderTarget != nullptr, "null is not accepted. Use setFinalRenderTargetToBackbuffer() for backbuffer");
		CHECKF(inFinalRenderTarget->isDepthFormat() == false, "Depth format is not supported yet");
		finalRenderTarget = inFinalRenderTarget;
	}

	void SceneRenderer::setFinalRenderTargetToBackbuffer() {
		finalRenderTarget = 0;
	}

	void SceneRenderer::reallocateSceneRenderTargets(RenderCommandList& cmdList, bool bEnableResolutionScaling) {
		sceneRenderTargets.reallocSceneTextures(cmdList, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight, bEnableResolutionScaling);

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

	void SceneRenderer::destroySceneRenderTargets(RenderCommandList& cmdList) {
		sceneRenderTargets.freeSceneTextures(cmdList);
		gRenderDevice->deleteFramebuffers(1, &gbufferFBO);
	}

	void SceneRenderer::render(RenderCommandList& cmdList, SceneProxy* inScene, Camera* inCamera) {
		scene = inScene;
		camera = inCamera;

		CHECK(sceneRenderSettings.isValid());

		bool bEnableResolutionScaling = (scene->sceneProxySource == SceneProxySource::MainScene);

		cmdList.sceneProxy = inScene;
		cmdList.sceneRenderTargets = &sceneRenderTargets;
		reallocateSceneRenderTargets(cmdList, bEnableResolutionScaling);

		// Prepare fallback material.
		if (fallbackMaterial.get() == nullptr) {
			fallbackMaterial = std::unique_ptr<Material>(Material::createMaterialInstance("solid_color"));
			fallbackMaterial->setConstantParameter("albedo", vector3(0.5f, 0.5f, 0.5f));
			fallbackMaterial->setConstantParameter("metallic", 0.0f);
			fallbackMaterial->setConstantParameter("roughness", 0.0f);
			fallbackMaterial->setConstantParameter("emissive", vector3(0.0f));
		}

		// #todo-multivew
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
				0, // mip
				GL_RGBA,
				GL_FLOAT,
				clearValues);
		}

		{
			// #todo-gpu-counter: Sky rendering cost should not be included here (or support nested counters)
			SCOPED_GPU_COUNTER(DirectLighting);
			renderDirectLighting(cmdList);
		}

		if (cvar_enable_probegi.getInt() != 0) {
			SCOPED_GPU_COUNTER(IndirectLighting);

			indirectLightingPass->renderIndirectLighting(cmdList, scene, camera, fullscreenQuad.get());
		}

		resolveUnlitPass->renderUnlit(cmdList, fullscreenQuad.get());

		if (cvar_enable_ssr.getInt() != 0) {
			SCOPED_GPU_COUNTER(ScreenSpaceReflection);
			screenSpaceReflectionPass->renderScreenSpaceReflection(cmdList, scene, camera, fullscreenQuad.get());
		}

		// Translucency pass
		{
			SCOPED_GPU_COUNTER(Translucency);
			translucency_pass->renderTranslucency(cmdList, scene, camera);
		}

		//////////////////////////////////////////////////////////////////////////
		// Post-processing
		GLuint sceneAfterLastPP = 0;

		if (sceneRenderSettings.enablePostProcess == false)
		{
			SCOPED_DRAW_EVENT(BlitToFinalTarget);
			copyTexture(cmdList, sceneRenderTargets.sceneColor, getFinalRenderTarget(),
				sceneRenderTargets.unscaledSceneWidth, sceneRenderTargets.unscaledSceneHeight);
		}
		else
		{
			// #todo: Support nested gpu counters
			SCOPED_GPU_COUNTER(PostProcessing);

			const EAntiAliasingMethod antiAliasing = (EAntiAliasingMethod)badger::clamp(0, cvar_anti_aliasing.getInt(), (int32)EAntiAliasingMethod::NumMethods);
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

			sceneAfterLastPP = sceneRenderTargets.sceneColor;

			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);

			// Make half res version of sceneColor. A common source for PPs that are too expensive to run in full res.
			const bool bNeedsHalfResSceneColor = isPPFinal(EPostProcessOrder::Bloom); // NOTE: Add other conditions if needed.
			if (bNeedsHalfResSceneColor) {
				SCOPED_DRAW_EVENT(SceneColorDownsample);
				copyTexture(cmdList, sceneRenderTargets.sceneColor, sceneRenderTargets.sceneColorHalfRes,
					sceneRenderTargets.sceneWidth / 2, sceneRenderTargets.sceneHeight / 2);
			}

			// Post Process: Bloom
			if (isPPEnabled(EPostProcessOrder::Bloom)) {
				bloomSetup->setInput(EPostProcessInput::PPI_0, sceneRenderTargets.sceneColorHalfRes);
				bloomSetup->setOutput(EPostProcessOutput::PPO_0, sceneRenderTargets.sceneBloomSetup);
				bloomSetup->renderPostProcess(cmdList, fullscreenQuad.get());

				bloomPass->setInput(EPostProcessInput::PPI_0, sceneRenderTargets.sceneBloomSetup);
				bloomPass->renderPostProcess(cmdList, fullscreenQuad.get());
			}

			// Post Process: Tone Mapping
			{
				const bool isFinalPP = isPPFinal(EPostProcessOrder::ToneMapping);

				GLuint bloom = isPPEnabled(EPostProcessOrder::Bloom) ? sceneRenderTargets.sceneBloomChain : sceneAfterLastPP;
				GLuint toneMappingRenderTarget = isFinalPP ? sceneRenderTargets.sceneFinal : sceneRenderTargets.sceneColorToneMapped;

				toneMapping->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
				toneMapping->setInput(EPostProcessInput::PPI_1, bloom);
				toneMapping->setInput(EPostProcessInput::PPI_2, sceneRenderTargets.godRayResult);
				toneMapping->setInput(EPostProcessInput::PPI_3, sceneRenderTargets.getVolumetricCloud(frameCounter));
				toneMapping->setOutput(EPostProcessOutput::PPO_0, toneMappingRenderTarget);
				toneMapping->renderPostProcess(cmdList, fullscreenQuad.get());

				sceneAfterLastPP = toneMappingRenderTarget;
			}

			// Post Process: Anti-aliasing
			switch (antiAliasing) {
			case EAntiAliasingMethod::NoAA:
				// Do nothing
				break;

			case EAntiAliasingMethod::FXAA:
				{
					const bool isFinalPP = isPPFinal(EPostProcessOrder::AntiAliasing);
					const GLuint aaRenderTarget = isFinalPP ? sceneRenderTargets.sceneFinal : sceneRenderTargets.sceneColorAA;

					fxaa->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
					fxaa->setOutput(EPostProcessOutput::PPO_0, aaRenderTarget);
					fxaa->renderPostProcess(cmdList, fullscreenQuad.get());

					sceneAfterLastPP = aaRenderTarget;
				}
				break;

			default:
				break;
			}

			// Super resolution
			// NOTE: Should run after tone mapping and before any noise-introducing PPs.
			if (isPPEnabled(EPostProcessOrder::SuperResolution)) {
				switch (superResMethod) {
				case ESuperResolutionMethod::FSR1:
					{
						const bool isFinalPP = isPPFinal(EPostProcessOrder::SuperResolution);
						const GLuint renderTarget = isFinalPP ? sceneRenderTargets.sceneFinal : sceneRenderTargets.sceneColorUpscaled;

						fsr1->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
						fsr1->setOutput(EPostProcessOutput::PPO_0, renderTarget);
						fsr1->renderPostProcess(cmdList, fullscreenQuad.get());

						sceneAfterLastPP = renderTarget;
					}
					break;

				default:
					break;
					
				}
			}

			// Post Process: Depth of Field
			if (isPPEnabled(EPostProcessOrder::DepthOfField)) {
				const GLuint dofInput = sceneRenderTargets.sceneColorDoFInput;
				const GLuint dofRenderTarget = sceneRenderTargets.sceneFinal;

				// Force rgba32f input.
				// #todo-dof: Don't copy and use sceneAfterLastPP directly if it's rgba32f.
				copyTexture(cmdList, sceneAfterLastPP, dofInput,
					sceneRenderTargets.sceneWidthSuperRes, sceneRenderTargets.sceneHeightSuperRes);

				depthOfField->setInput(EPostProcessInput::PPI_0, dofInput);
				depthOfField->setOutput(EPostProcessOutput::PPO_0, dofRenderTarget);
				depthOfField->renderPostProcess(cmdList, fullscreenQuad.get());

				sceneAfterLastPP = dofRenderTarget;
			}

		}

		// Assumes rendering result is always written to sceneFinal and it's pixel format is rgba16f.
		if (scene->bScreenshotReserved) {
			cmdList.bindFramebuffer(GL_READ_FRAMEBUFFER, fboScreenshot);
			cmdList.namedFramebufferTexture(fboScreenshot, GL_COLOR_ATTACHMENT0, sceneRenderTargets.sceneFinal, 0);
			cmdList.namedFramebufferReadBuffer(fboScreenshot, GL_COLOR_ATTACHMENT0);

			scene->screenshotSize = vector2i((int32)sceneRenderTargets.sceneWidth, (int32)sceneRenderTargets.sceneHeight);
			scene->screenshotRawData.resize(4 * sceneRenderTargets.sceneWidth * sceneRenderTargets.sceneHeight);
			cmdList.pixelStorei(GL_PACK_ALIGNMENT, 1);
			cmdList.readPixels(0, 0, sceneRenderTargets.sceneWidth, sceneRenderTargets.sceneHeight, GL_RGBA, GL_HALF_FLOAT, scene->screenshotRawData.data());
			cmdList.pixelStorei(GL_PACK_ALIGNMENT, 4);
		}

		if (sceneAfterLastPP != 0) {
			SCOPED_DRAW_EVENT(BlitToFinalTarget);
			copyTexture(cmdList, sceneAfterLastPP, getFinalRenderTarget(),
				sceneRenderTargets.unscaledSceneWidth, sceneRenderTargets.unscaledSceneHeight);
		}

		// Debug pass (r.viewmode)
		visualizeBuffer->render(cmdList, scene, camera);

		scene = nullptr;
		camera = nullptr;
	}

	void SceneRenderer::renderBasePass(RenderCommandList& cmdList) {
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
		cmdList.viewport(0, 0, sceneRenderTargets.sceneWidth, sceneRenderTargets.sceneHeight);

		// #todo-depthprepass: GEQUAL or LEQUAL as I'm not doing full-depth prepass.
		// Switch to EQUAL when doing full-depth prepass.
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

		{
			const std::vector<StaticMeshProxy*>& proxyList = scene->getOpaqueStaticMeshes();
			const size_t numProxies = proxyList.size();
			uint32 currentProgramHash = 0;
			uint32 currentMIID = 0xffffffff;

			for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
				StaticMeshProxy* proxy = proxyList[proxyIx];
				Material* material = proxy->material;
				MaterialShader* materialShader = material->internal_getMaterialShader();

				// Early out
				if (bEnableFrustumCulling && !proxy->bInFrustum) {
					continue;
				}

				bool bShouldBindProgram = (currentProgramHash != materialShader->programHash);
				bool bShouldUpdateMaterialParameters = bShouldBindProgram || (currentMIID != material->internal_getMaterialInstanceID());
				bool bUseWireframeMode = material->bWireframe;
				currentProgramHash = materialShader->programHash;
				currentMIID = material->internal_getMaterialInstanceID();

				if (bShouldBindProgram) {
					SCOPED_DRAW_EVENT(BindMaterialProgram);

					uint32 programName = materialShader->program->getGLName();
					CHECK(programName != 0 && programName != 0xffffffff);
					cmdList.useProgram(programName);
				}

				// Update UBO (per object)
				{
					Material::UBO_PerObject uboData;
					uboData.modelTransform = proxy->modelMatrix;
					uboPerObject.update(cmdList, Material::UBO_PerObject::BINDING_POINT, &uboData);
				}

				// Update UBO (material)
				if (bShouldUpdateMaterialParameters && materialShader->uboTotalBytes > 0) {
					uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(materialShader->uboTotalBytes));
					material->internal_fillUniformBuffer(uboMemory);
					materialShader->uboMaterial.update(cmdList, materialShader->uboBindingPoint, uboMemory);
				}

				// Bind texture units
				if (bShouldUpdateMaterialParameters) {
					for (const MaterialTextureParameter& mtp : material->internal_getTextureParameters()) {
						cmdList.bindTextureUnit(mtp.binding, mtp.glTexture);
					}
				}

				// #todo-renderer: Batching by same state
				if (proxy->doubleSided || bUseWireframeMode) cmdList.disable(GL_CULL_FACE);
				if (proxy->renderInternal) cmdList.frontFace(GL_CW);
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

				proxy->geometry->activate_position_uv_normal_tangent_bitangent(cmdList);
				proxy->geometry->activateIndexBuffer(cmdList);
				proxy->geometry->drawPrimitive(cmdList);

				// #todo-renderer: Batching by same state
				if (proxy->doubleSided || bUseWireframeMode) cmdList.enable(GL_CULL_FACE);
				if (proxy->renderInternal) cmdList.frontFace(GL_CCW);
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		cmdList.depthMask(GL_TRUE);
	}

	void SceneRenderer::renderDirectLighting(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(DirectLighting);

		directLightingPass->bindFramebuffer(cmdList);
		directLightingPass->renderDirectLighting(cmdList, scene, camera);

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
	}

	void SceneRenderer::copyTexture(RenderCommandList& cmdList, GLuint source,
		GLuint target, uint32 targetWidth, uint32 targetHeight)
	{
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_CopyTexture);

		cmdList.useProgram(program.getGLName());
		if (target == 0) {
			cmdList.bindFramebuffer(GL_FRAMEBUFFER, 0);
		} else {
			cmdList.bindFramebuffer(GL_FRAMEBUFFER, SceneRenderer::copyTextureFBO);
			cmdList.namedFramebufferTexture(SceneRenderer::copyTextureFBO, GL_COLOR_ATTACHMENT0, target, 0);
		}
		cmdList.viewport(0, 0, targetWidth, targetHeight);
		cmdList.bindTextureUnit(0, source);
		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
		fullscreenQuad->deactivate(cmdList);
		fullscreenQuad->deactivateIndexBuffer(cmdList);
	}

	GLuint SceneRenderer::getFinalRenderTarget() const {
		if (finalRenderTarget == nullptr) {
			return 0; // Default backbuffer
		}
		return finalRenderTarget->getGLName();
	}

	void SceneRenderer::updateSceneUniformBuffer(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera)
	{
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

		data.screenResolution.x = (float)sceneRenderTargets.sceneWidth;
		data.screenResolution.y = (float)sceneRenderTargets.sceneHeight;
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

		// Regard first directional light as Sun.
		data.sunExists = scene->proxyList_directionalLight.size() > 0;
		data.sunLight = *(scene->proxyList_directionalLight[0]);

		ubo_perFrame->update(cmdList, SCENE_UNIFORM_BINDING_INDEX, &data);

		prevView = data.view;
		prevInverseView = data.inverseView;
	}

}

namespace pathos {
	
	std::unique_ptr<class Material>                SceneRenderer::fallbackMaterial;
	std::unique_ptr<class PlaneGeometry>           SceneRenderer::fullscreenQuad;
	GLuint                                         SceneRenderer::copyTextureFBO = 0;
	
	std::unique_ptr<UniformBuffer>                 SceneRenderer::ubo_perFrame;
	
	std::unique_ptr<DirectLightingPass>            SceneRenderer::directLightingPass;
	std::unique_ptr<IndirectLightingPass>          SceneRenderer::indirectLightingPass;
	std::unique_ptr<ScreenSpaceReflectionPass>     SceneRenderer::screenSpaceReflectionPass;

	std::unique_ptr<ResolveUnlitPass>              SceneRenderer::resolveUnlitPass;

	std::unique_ptr<class TranslucencyRendering>   SceneRenderer::translucency_pass;

	std::unique_ptr<class SkyboxPass>              SceneRenderer::skyboxPass;
	std::unique_ptr<class AnselSkyPass>            SceneRenderer::anselSkyPass;
	std::unique_ptr<class SkyAtmospherePass>       SceneRenderer::skyAtmospherePass;
	std::unique_ptr<class VolumetricCloudPass>     SceneRenderer::volumetricCloud;

	std::unique_ptr<class DepthPrepass>            SceneRenderer::depthPrepass;
	std::unique_ptr<DirectionalShadowMap>          SceneRenderer::sunShadowMap;
	std::unique_ptr<OmniShadowPass>                SceneRenderer::omniShadowPass;
	std::unique_ptr<class VisualizeBufferPass>     SceneRenderer::visualizeBuffer;

	std::unique_ptr<class GodRay>                  SceneRenderer::godRay;
	std::unique_ptr<class SSAO>                    SceneRenderer::ssao;
	std::unique_ptr<class BloomSetup>              SceneRenderer::bloomSetup;
	std::unique_ptr<class BloomPass>               SceneRenderer::bloomPass;
	std::unique_ptr<class ToneMapping>             SceneRenderer::toneMapping;
	std::unique_ptr<class FXAA>                    SceneRenderer::fxaa;
	std::unique_ptr<class FSR1>                    SceneRenderer::fsr1;
	std::unique_ptr<class DepthOfField>            SceneRenderer::depthOfField;

	void SceneRenderer::internal_initGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
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
			directLightingPass = std::make_unique<DirectLightingPass>();
			indirectLightingPass = std::make_unique<IndirectLightingPass>();
			screenSpaceReflectionPass = std::make_unique<ScreenSpaceReflectionPass>();
			translucency_pass = std::make_unique<TranslucencyRendering>();

			directLightingPass->initializeResources(cmdList);
			indirectLightingPass->initializeResources(cmdList);
			screenSpaceReflectionPass->initializeResources(cmdList);
			translucency_pass->initializeResources(cmdList);
		}

		resolveUnlitPass = std::make_unique<ResolveUnlitPass>();
		resolveUnlitPass->initializeResources(cmdList);

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
			fsr1 = pathos::makeUnique<FSR1>();
			depthOfField = std::make_unique<DepthOfField>();

			godRay->initializeResources(cmdList);
			ssao->initializeResources(cmdList);
			bloomSetup->initializeResources(cmdList);
			bloomPass->initializeResources(cmdList);
			toneMapping->initializeResources(cmdList);
			fxaa->initializeResources(cmdList);
			fsr1->initializeResources(cmdList);
			depthOfField->initializeResources(cmdList);
		}
	}

	void SceneRenderer::internal_destroyGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
		fallbackMaterial.release();
		fullscreenQuad->dispose();
		gRenderDevice->deleteBuffers(1, &copyTextureFBO);

		ubo_perFrame.release();

#define DESTROYPASS(pass) { pass->destroyResources(cmdList); pass.reset(); }
#define RELEASEPASS(pass) { pass->releaseResources(cmdList); pass.reset(); }

		DESTROYPASS(directLightingPass);
		DESTROYPASS(indirectLightingPass);
		DESTROYPASS(screenSpaceReflectionPass);
		RELEASEPASS(translucency_pass);

		DESTROYPASS(resolveUnlitPass);

		DESTROYPASS(skyboxPass);
		DESTROYPASS(anselSkyPass);
		DESTROYPASS(skyAtmospherePass);
		DESTROYPASS(volumetricCloud);

		DESTROYPASS(depthPrepass);
		DESTROYPASS(sunShadowMap);
		DESTROYPASS(omniShadowPass);
		DESTROYPASS(visualizeBuffer);

		RELEASEPASS(godRay);
		RELEASEPASS(ssao);
		RELEASEPASS(bloomSetup);
		RELEASEPASS(bloomPass);
		RELEASEPASS(toneMapping);
		RELEASEPASS(fxaa);
		RELEASEPASS(fsr1);
		RELEASEPASS(depthOfField);

#undef DESTROYPASS
#undef RELEASEPASS

		cmdList.flushAllCommands();
	}

	DEFINE_GLOBAL_RENDER_ROUTINE(SceneRenderer, SceneRenderer::internal_initGlobalResources, SceneRenderer::internal_destroyGlobalResources);

}
