#pragma once

#include "pathos/render/renderer.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/deferred/deferredpass.h"
#include "pathos/render/postprocessing/anti_aliasing.h"
#include "pathos/shader/uniform_buffer.h"
#include "pathos/camera/camera.h"
#include "pathos/light/shadow_directional.h"
#include "pathos/light/shadow_omni.h"

#include <memory>

namespace pathos {

	class OpenGLDevice;
	class RenderTarget2D;

	class DeferredRenderer : public Renderer {

	public:
		static void internal_initGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
		static void internal_destroyGlobalResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
	private:
		static std::unique_ptr<class ColorMaterial> fallbackMaterial;
		static std::unique_ptr<class PlaneGeometry> fullscreenQuad;
		static GLuint copyTextureFBO; // for DeferredRenderer::copyTexture()

		static std::unique_ptr<UniformBuffer> ubo_perFrame;

		// mesh rendering
		static MeshDeferredRenderPass_Pack* pack_passes[static_cast<uint32>(MATERIAL_ID::NUM_MATERIAL_IDS)];
		static std::unique_ptr<MeshDeferredRenderPass_Unpack> unpack_pass;
		static std::unique_ptr<class TranslucencyRendering> translucency_pass;

		// sky & atmosphere
		static std::unique_ptr<class SkyboxPass>          skyboxPass;
		static std::unique_ptr<class AnselSkyPass>        anselSkyPass;
		static std::unique_ptr<class SkyAtmospherePass>   skyAtmospherePass;
		static std::unique_ptr<class VolumetricCloudPass> volumetricCloud;

		// full-screen processing
		static std::unique_ptr<class DepthPrepass> depthPrepass;
		static std::unique_ptr<DirectionalShadowMap> sunShadowMap;
		static std::unique_ptr<OmniShadowPass> omniShadowPass;
		static std::unique_ptr<class VisualizeBufferPass> visualizeBuffer;

		// post-processing
		static std::unique_ptr<class GodRay> godRay;
		static std::unique_ptr<class SSAO> ssao;
		static std::unique_ptr<class BloomSetup> bloomSetup;
		static std::unique_ptr<class BloomPass> bloomPass;
		static std::unique_ptr<class ToneMapping> toneMapping;
		static std::unique_ptr<class FXAA> fxaa;
		static std::unique_ptr<class DepthOfField> depthOfField;

	public:
		DeferredRenderer();
		virtual ~DeferredRenderer();

		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;

		virtual void setSceneRenderSettings(const SceneRenderSettings& settings) override;
		virtual void setFinalRenderTarget(RenderTarget2D* finalRenderTarget) override;
		virtual void setFinalRenderTargetToBackbuffer() override;
		virtual void render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) override;

		// #todo: Make as a utility function, not a method of renderer.
		void copyTexture(RenderCommandList& cmdList, GLuint source, GLuint target);

	private:
		void reallocateSceneRenderTargets(RenderCommandList& cmdList);
		void destroySceneRenderTargets(RenderCommandList& cmdList);

		void updateSceneUniformBuffer(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

		void clearGBuffer(RenderCommandList& cmdList);
		void packGBuffer(RenderCommandList& cmdList);
		void unpackGBuffer(RenderCommandList& cmdList);

		void renderTranslucency(RenderCommandList& cmdList);

		GLuint getFinalRenderTarget() const;

	private:
		bool destroyed = false;
		uint32 frameCounter = 0;

		EAntiAliasingMethod antiAliasing;

		// #todo-renderer: Implement render target pool
		SceneRenderTargets sceneRenderTargets;
		GLuint gbufferFBO = 0;

		SceneRenderSettings sceneRenderSettings;
		RenderTarget2D* finalRenderTarget = nullptr;

		// temporary save
		SceneProxy* scene;
		Camera* camera;
		matrix4 prevView;
		matrix4 prevInverseView;

	};

}
