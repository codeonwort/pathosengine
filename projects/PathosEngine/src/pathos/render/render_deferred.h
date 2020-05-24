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

	class RenderTarget2D;

	class DeferredRenderer : public Renderer {

	public:
		DeferredRenderer(uint32 width, uint32 height);
		virtual ~DeferredRenderer();

		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void render(RenderCommandList& cmdList, Scene* scene, Camera* camera) override;
		virtual void setFinalRenderTarget(RenderTarget2D* finalRenderTarget) override;

	private:
		// #todo-scene-capture: Global resources should not be owned by renderer
		void createShaders();
		void destroyShaders();

		void reallocateSceneRenderTargets(RenderCommandList& cmdList);
		void destroySceneRenderTargets(RenderCommandList& cmdList);

		void updateSceneUniformBuffer(RenderCommandList& cmdList, Scene* scene, Camera* camera);

		void clearGBuffer(RenderCommandList& cmdList);
		void packGBuffer(RenderCommandList& cmdList);
		void unpackGBuffer(RenderCommandList& cmdList);

		void renderTranslucency(RenderCommandList& cmdList);

		GLuint getFinalRenderTarget() const;

	private:
		bool destroyed = false;

		EAntiAliasingMethod antiAliasing;

		SceneRenderTargets sceneRenderTargets;
		GLuint gbufferFBO = 0;

		UniformBuffer ubo_perFrame;

		std::unique_ptr<class ColorMaterial> fallbackMaterial;

		MeshDeferredRenderPass_Pack* pack_passes[static_cast<uint32>(MATERIAL_ID::NUM_MATERIAL_IDS)];
		MeshDeferredRenderPass_Unpack* unpack_pass;
		std::unique_ptr<class TranslucencyRendering> translucency_pass;

		std::unique_ptr<DirectionalShadowMap> sunShadowMap;

		class VisualizeDepth* visualizeDepth;

		// post-processing
		std::unique_ptr<class GodRay> godRay;
		std::unique_ptr<class SSAO> ssao;
		std::unique_ptr<class BloomPass> bloomPass;
		std::unique_ptr<class ToneMapping> toneMapping;
		std::unique_ptr<class FXAA> fxaa;
		std::unique_ptr<class DepthOfField> depthOfField;

		std::unique_ptr<class PlaneGeometry> fullscreenQuad;

		RenderTarget2D* finalRenderTarget = nullptr;

		// temporary save
		Scene* scene; 
		Camera* camera;
		uint32 sceneWidth;
		uint32 sceneHeight;

	};

}
