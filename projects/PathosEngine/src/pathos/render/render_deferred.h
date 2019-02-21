#pragma once

#include "renderer.h"
#include "deferred/deferredpass.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"
#include "pathos/light/shadow_directional.h"

namespace pathos {

	// deferred renderer implementation
	class DeferredRenderer : public Renderer {

	public:
		DeferredRenderer(unsigned int width, unsigned int height);
		DeferredRenderer(const DeferredRenderer& other) = delete;
		DeferredRenderer(DeferredRenderer&&) = delete;
		virtual ~DeferredRenderer();

		void render(Scene*, Camera*); // render the total scene

		inline void setHDR(bool value) { useHDR = value; }

		GLuint debug_godRayTexture() { return unpack_pass->debug_godRayTexture(); }

	private:
		void createShaders();
		void destroyShaders();

		void createGBuffer();
		void destroyGBuffer();

		void createUBO();
		void destroyUBO();

		void updateUBO(Scene* scene, Camera* camera);
		void clearGBuffer();
		void packGBuffer();
		void unpackGBuffer();

		void renderSkybox(Skybox*);

	private:
		bool useHDR = false;

		GLuint fbo; // g-buffer
		GLuint fbo_attachment[4]; // textures attached to g-buffer (0, 1, 2 for color. 3 for depth/stencil)
		GLsizei width, height; // fbo texture size
		GLenum draw_buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

		GLuint ubo_perFrame;

		MeshDeferredRenderPass_Pack* pack_passes[(int)MATERIAL_ID::NUM_MATERIAL_IDS];
		MeshDeferredRenderPass_Unpack* unpack_pass;

		class VisualizeDepth* visualizeDepth;
		DirectionalShadowMap* sunShadowMap;

		Scene* scene; // temporary save. actually don't need
		Camera* camera; // temporary save. actually don't need

	};

}
