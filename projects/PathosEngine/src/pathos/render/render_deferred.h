#pragma once

#include "renderer.h"
#include "deferred/deferredpass.h"
#include "pathos/shader/uniform_buffer.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"
#include "pathos/light/shadow_directional.h"

namespace pathos {

	class DeferredRenderer : public Renderer {

	// Public API
	public:
		DeferredRenderer(unsigned int width, unsigned int height);
		virtual ~DeferredRenderer();

		virtual void render(Scene*, Camera*) override;

		inline void setHDR(bool value) { useHDR = value; }

	// Debug API
	public:
		inline GLuint debug_godRayTexture() const { return unpack_pass->debug_godRayTexture(); }
		inline DirectionalShadowMap* debug_getShadowMap() const { return sunShadowMap; }

	private:
		void createShaders();
		void destroyShaders();

		void createGBuffer();
		void destroyGBuffer();

		void updateUBO(Scene* scene, Camera* camera);
		void clearGBuffer();
		void packGBuffer();
		void unpackGBuffer();

	private:
		bool useHDR = true;

		// #todo-renderer: Manage render targets in separate structure
		GLuint fbo; // g-buffer
		GLuint fbo_attachment[4]; // textures attached to g-buffer (0, 1, 2 for color. 3 for depth/stencil)
		GLsizei width, height; // fbo texture size
		GLenum draw_buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

		UniformBuffer ubo_perFrame;

		MeshDeferredRenderPass_Pack* pack_passes[(int)MATERIAL_ID::NUM_MATERIAL_IDS];
		MeshDeferredRenderPass_Unpack* unpack_pass;

		DirectionalShadowMap* sunShadowMap;

		class VisualizeDepth* visualizeDepth;

		Scene* scene; // temporary save. actually don't need
		Camera* camera; // temporary save. actually don't need

	};

}
