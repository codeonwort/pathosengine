#pragma once

#include "renderer.h"
#include "scene_render_targets.h"
#include "deferred/deferredpass.h"
#include "postprocessing/anti_aliasing.h"
#include "pathos/shader/uniform_buffer.h"
#include "pathos/camera/camera.h"
#include "pathos/scene/scene.h"
#include "pathos/light/shadow_directional.h"

#include <memory>

namespace pathos {

	class OpenGLDevice;
	class RenderTarget2D;

	class DeferredRenderer : public Renderer {

	public:
		static void internal_initGlobalResources(OpenGLDevice* renderDevice);
		static void internal_destroyGlobalResources(OpenGLDevice* renderDevice);
	private:
		static std::unique_ptr<class ColorMaterial> fallbackMaterial;
		static std::unique_ptr<class PlaneGeometry> fullscreenQuad;
		static GLuint copyTextureFBO; // for DeferredRenderer::copyTexture()

		static std::unique_ptr<UniformBuffer> ubo_perFrame;

		// mesh rendering
		static MeshDeferredRenderPass_Pack* pack_passes[static_cast<uint32>(MATERIAL_ID::NUM_MATERIAL_IDS)];
		static std::unique_ptr<MeshDeferredRenderPass_Unpack> unpack_pass;
		static std::unique_ptr<class TranslucencyRendering> translucency_pass;

		// full-screen processing
		static std::unique_ptr<DirectionalShadowMap> sunShadowMap;
		static std::unique_ptr<class VisualizeDepth> visualizeDepth;

		// post-processing
		static std::unique_ptr<class GodRay> godRay;
		static std::unique_ptr<class SSAO> ssao;
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
		virtual void render(RenderCommandList& cmdList, Scene* scene, Camera* camera) override;

	private:
		void reallocateSceneRenderTargets(RenderCommandList& cmdList);
		void destroySceneRenderTargets(RenderCommandList& cmdList);

		void updateSceneUniformBuffer(RenderCommandList& cmdList, Scene* scene, Camera* camera);

		void clearGBuffer(RenderCommandList& cmdList);
		void packGBuffer(RenderCommandList& cmdList);
		void unpackGBuffer(RenderCommandList& cmdList);

		void renderTranslucency(RenderCommandList& cmdList);

		void copyTexture(RenderCommandList& cmdList, GLuint source, GLuint target);

		GLuint getFinalRenderTarget() const;

	private:
		bool destroyed = false;

		EAntiAliasingMethod antiAliasing;

		// #todo-scene-capture: Are these global resources?
		SceneRenderTargets sceneRenderTargets;
		GLuint gbufferFBO = 0;

		SceneRenderSettings sceneRenderSettings;
		RenderTarget2D* finalRenderTarget = nullptr;

		// temporary save
		Scene* scene; 
		Camera* camera;

	};

}
