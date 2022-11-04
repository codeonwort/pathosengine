#pragma once

#include "pathos/render/renderer.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/direct_lighting.h"
#include "pathos/render/indirect_lighting.h"
#include "pathos/render/resolve_unlit.h"
#include "pathos/render/screen_space_reflection.h"
#include "pathos/render/postprocessing/anti_aliasing.h"
#include "pathos/shader/uniform_buffer.h"
#include "pathos/camera/camera.h"
#include "pathos/smart_pointer.h"

#include <memory>

namespace pathos {

	class OpenGLDevice;
	class RenderTarget2D;

	class SceneRenderer : public Renderer {

	public:
		static void internal_initGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
		static void internal_destroyGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);

	private:
		static uniquePtr<class Material> fallbackMaterial;
		static uniquePtr<class PlaneGeometry> fullscreenQuad;
		static GLuint copyTextureFBO; // for DeferredRenderer::copyTexture()

		static uniquePtr<UniformBuffer> ubo_perFrame;

		// Local & global illumination
		static uniquePtr<DirectLightingPass>          directLightingPass;
		static uniquePtr<IndirectLightingPass>        indirectLightingPass;
		static uniquePtr<ScreenSpaceReflectionPass>   screenSpaceReflectionPass;

		// Unlit
		static uniquePtr<ResolveUnlitPass>            resolveUnlitPass;

		// Sky & atmosphere
		static uniquePtr<class SkyboxPass>            skyboxPass;
		static uniquePtr<class AnselSkyPass>          anselSkyPass;
		static uniquePtr<class SkyAtmospherePass>     skyAtmospherePass;
		static uniquePtr<class VolumetricCloudPass>   volumetricCloud;

		// Translucency
		static uniquePtr<class TranslucencyRendering> translucency_pass;

		// Full-screen processing
		static uniquePtr<class DepthPrepass>          depthPrepass;
		static uniquePtr<class DirectionalShadowMap>  sunShadowMap;
		static uniquePtr<class OmniShadowPass>        omniShadowPass;
		static uniquePtr<class VisualizeBufferPass>   visualizeBuffer;

		// Post-processing
		static uniquePtr<class GodRay>                godRay;
		static uniquePtr<class SSAO>                  ssao;
		static uniquePtr<class BloomSetup>            bloomSetup;
		static uniquePtr<class BloomPass>             bloomPass;
		static uniquePtr<class ToneMapping>           toneMapping;
		static uniquePtr<class FXAA>                  fxaa;
		static uniquePtr<class TAA>                   taa;
		static uniquePtr<class FSR1>                  fsr1;
		static uniquePtr<class DepthOfField>          depthOfField;

	public:
		SceneRenderer();
		virtual ~SceneRenderer();

		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;

		virtual void setSceneRenderSettings(const SceneRenderSettings& settings) override;
		virtual void setFinalRenderTarget(RenderTarget2D* finalRenderTarget) override;
		virtual void setFinalRenderTargetToBackbuffer() override;
		virtual void render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) override;

		// #todo: Make as a utility function, not a method of renderer.
		void copyTexture(RenderCommandList& cmdList, GLuint source,
			GLuint target, uint32 targetWidth, uint32 targetHeight);

	private:
		void reallocateSceneRenderTargets(RenderCommandList& cmdList, bool bEnableResolutionScaling);
		void destroySceneRenderTargets(RenderCommandList& cmdList);

		void updateSceneUniformBuffer(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

		void renderBasePass(RenderCommandList& cmdList);

		void renderDirectLighting(RenderCommandList& cmdList);

		GLuint getFinalRenderTarget() const;

	private:
		bool destroyed = false;
		uint32 frameCounter = 0;

		// #todo-renderer: Implement render target pool
		SceneRenderTargets sceneRenderTargets;
		GLuint gbufferFBO = 0;

		UniformBuffer uboPerObject;

		SceneRenderSettings sceneRenderSettings;
		RenderTarget2D* finalRenderTarget = nullptr;

		GLuint fboScreenshot = 0; // Dummy FBO to read screenshot.

		// temporary save
		SceneProxy* scene = nullptr;
		Camera* camera = nullptr;
		matrix4 prevView;
		matrix4 prevInverseView;
		matrix4 prevViewProj;

		static constexpr uint32 JITTER_SEQ_LENGTH = 16;
		float temporalJitterSequenceX[JITTER_SEQ_LENGTH];
		float temporalJitterSequenceY[JITTER_SEQ_LENGTH];
	};

}
