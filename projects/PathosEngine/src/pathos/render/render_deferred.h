#pragma once

#include "renderer.h"
#include "scene_render_targets.h"
#include "deferred/deferredpass.h"
#include "pathos/shader/uniform_buffer.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"
#include "pathos/light/shadow_directional.h"

namespace pathos {

	class DeferredRenderer : public Renderer {

	public:
		DeferredRenderer(uint32 width, uint32 height);
		virtual ~DeferredRenderer();

		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void render(RenderCommandList& cmdList, Scene*, Camera*) override;

		inline void setHDR(bool value) { useHDR = value; }

	private:
		void createShaders();
		void destroyShaders();

		void reallocateSceneRenderTargets(RenderCommandList& cmdList);
		void destroySceneRenderTargets(RenderCommandList& cmdList);

		void updateSceneUniformBuffer(RenderCommandList& cmdList, Scene* scene, Camera* camera);
		void clearGBuffer(RenderCommandList& cmdList);
		void packGBuffer(RenderCommandList& cmdList);
		void unpackGBuffer(RenderCommandList& cmdList);

	private:
		bool useHDR = true;
		bool destroyed = false;

		SceneRenderTargets sceneRenderTargets;
		GLuint gbufferFBO = 0;

		UniformBuffer ubo_perFrame;

		MeshDeferredRenderPass_Pack* pack_passes[(int)MATERIAL_ID::NUM_MATERIAL_IDS];
		MeshDeferredRenderPass_Unpack* unpack_pass;

		DirectionalShadowMap* sunShadowMap;

		class VisualizeDepth* visualizeDepth;

		// temporary save
		Scene* scene; 
		Camera* camera;
		uint32 sceneWidth;
		uint32 sceneHeight;

	};

}
