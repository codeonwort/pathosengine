#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/render/renderer.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/direct_lighting.h"
#include "pathos/render/indirect_lighting.h"
#include "pathos/render/resolve_unlit.h"
#include "pathos/render/screen_space_reflection.h"
#include "pathos/render/postprocessing/anti_aliasing.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/smart_pointer.h"

#include <memory>

namespace pathos {

	class OpenGLDevice;
	class RenderTargetView;

	struct UBO_PerFrame {
		static constexpr uint32 BINDING_POINT = 0;

		matrix4               view;
		matrix4               inverseView;
		matrix3x4             view3x3; // Name is 3x3, but type should be 3x4 due to how padding works in glsl
		matrix4               viewProj;
		matrix4               proj;
		matrix4               inverseProj;

		// For reprojection
		matrix4               prevView;
		matrix4               prevInverseView;
		matrix4               prevViewProj;

		vector4               projParams;
		vector4               temporalJitter; // (x, y, ?, ?)
		vector4               screenResolution; // (w, h, 1/w, 1/h)
		vector4               zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))
		vector4               time; // (currentTime, ?, ?, ?)

		matrix4               sunViewProj[4];
		vector4               sunParameters; // (CSM_zFar, numCascades, ?, ?)
		vector4               csmDepths;     // 4 cascades

		vector3               eyeDirection;
		float                 __pad0;

		vector3               eyePosition;
		float                 __pad1;

		vector3               ws_eyePosition;
		uint32                sunExists;

		DirectionalLightProxy sunLight;
	};

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
		virtual void setFinalRenderTarget(RenderTargetView* finalRenderTarget) override;
		virtual void setFinalRenderTargetToBackbuffer() override;
		virtual void renderScene(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) override;

		// #todo: Make as a utility function, not a method of renderer.
		void copyTexture(RenderCommandList& cmdList, GLuint source,
			GLuint target, uint32 targetWidth, uint32 targetHeight);

	private:
		void reallocateSceneRenderTargets(RenderCommandList& cmdList, bool bEnableResolutionScaling);
		void destroySceneRenderTargets(RenderCommandList& cmdList);

		void updateSceneUniformBuffer(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

		// Some render passes alter uboPerFrame as a dirty hack.
		void revertHackedSceneUniformBuffer(RenderCommandList& cmdList);

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
		RenderTargetView* finalRenderTarget = nullptr;

		GLuint fboScreenshot = 0; // Dummy FBO to read screenshot.

		// temporary save
		SceneProxy* scene = nullptr;
		Camera* camera = nullptr;
		matrix4 prevView;
		matrix4 prevInverseView;
		matrix4 prevViewProj;
		UBO_PerFrame cachedPerFrameUBOData;

		static constexpr uint32 JITTER_SEQ_LENGTH = 16;
		float temporalJitterSequenceX[JITTER_SEQ_LENGTH];
		float temporalJitterSequenceY[JITTER_SEQ_LENGTH];
	};

}
