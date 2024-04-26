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
		uint32                bReverseZ;

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
		static class MeshGeometry* fullscreenQuad;
		static uniquePtr<class Material> fallbackMaterial;
		static GLuint copyTextureFBO; // for DeferredRenderer::copyTexture()

		static uniquePtr<UniformBuffer> ubo_perFrame;

		// G-buffer rendering
		static uniquePtr<class DepthPrepass>            depthPrepass;
		static uniquePtr<class GBufferPass>             gbufferPass;
		static uniquePtr<ResolveUnlitPass>              resolveUnlitPass;

		// Shadowmap rendering
		static uniquePtr<class DirectionalShadowMap>    sunShadowMap;
		static uniquePtr<class OmniShadowPass>          omniShadowPass;

		// Local & global illumination
		static uniquePtr<DirectLightingPass>            directLightingPass;
		static uniquePtr<IndirectLightingPass>          indirectLightingPass;
		static uniquePtr<ScreenSpaceReflectionPass>     screenSpaceReflectionPass;

		// Sky & atmosphere
		static uniquePtr<class SkyboxPass>              skyboxPass;
		static uniquePtr<class PanoramaSkyPass>         panoramaSkyPass;
		static uniquePtr<class SkyAtmospherePass>       skyAtmospherePass;
		static uniquePtr<class VolumetricCloudPass>     volumetricCloud;

		// Translucency
		static uniquePtr<class TranslucencyRendering>   translucency_pass;

		// Auto exposure
		static uniquePtr<class AutoExposurePass>        autoExposurePass;

		// Debug rendering
		static uniquePtr<class VisualizeBufferPass>     visualizeBuffer;
		static uniquePtr<class VisualizeLightProbePass> visualizeLightProbe;

		// Post-processing
		static uniquePtr<class GodRay>                  godRay;
		static uniquePtr<class SSAO>                    ssao;
		static uniquePtr<class BloomSetup>              bloomSetup;
		static uniquePtr<class BloomPass>               bloomPass;
		static uniquePtr<class ToneMapping>             toneMapping;
		static uniquePtr<class FXAA>                    fxaa;
		static uniquePtr<class TAA>                     taa;
		static uniquePtr<class FSR1>                    fsr1;
		static uniquePtr<class DepthOfField>            depthOfField;

	public:
		SceneRenderer();
		virtual ~SceneRenderer();

		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;

		virtual void setSceneRenderSettings(const SceneRenderSettings& settings) override;
		virtual void setFinalRenderTarget(RenderTargetView* finalRenderTarget) override;
		virtual void setFinalRenderTargetToBackbuffer() override;

		virtual void renderScene(
			RenderCommandList& cmdList,
			SceneRenderTargets* sceneRenderTargets,
			SceneProxy* scene,
			Camera* camera) override;

		// #todo-renderer: Make as a utility function, not a method of renderer.
		enum class ECopyTextureMode { CopyColor = 0, LightProbeDepth = 1 };
		void copyTexture(
			RenderCommandList& cmdList, GLuint source,
			GLuint target, uint32 targetWidth, uint32 targetHeight,
			ECopyTextureMode copyMode = ECopyTextureMode::CopyColor);

	private:
		void updateSceneUniformBuffer(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

		GLuint getFinalRenderTarget() const;

	private:
		bool destroyed = false;
		uint32 frameCounter = 0;

		SceneRenderSettings sceneRenderSettings;
		RenderTargetView* finalRenderTarget = nullptr;

		GLuint fboScreenshot = 0; // Dummy FBO to read screenshot.

		// temporary save
		SceneRenderTargets* sceneRenderTargets = nullptr;
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
