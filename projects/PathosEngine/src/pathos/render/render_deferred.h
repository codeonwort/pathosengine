#pragma once

#include "deferred/deferredpass.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"

namespace pathos {

	// deferred renderer implementation
	class DeferredRenderer {

	public:
		DeferredRenderer(unsigned int width, unsigned int height);
		DeferredRenderer(const DeferredRenderer& other) = delete;
		DeferredRenderer(DeferredRenderer&&) = delete;
		virtual ~DeferredRenderer();

		void render(Scene* scene, Camera*); // render the total scene

		inline void setHDR(bool value) { useHDR = value; }

	protected:
		bool useHDR = false;

		void createShaders();
		void destroyShaders();

		void createGBuffer();
		void destroyGBuffer();
		GLuint fbo; // g-buffer
		GLuint fbo_attachment[3]; // textures attached to g-buffer (0, 1 for color. 2 for depth/stencil)
		GLsizei width, height; // fbo texture size
		GLenum draw_buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

		// invoked by render()
		void clearGBuffer();
		void packGBuffer();
		void unpackGBuffer();
		Scene* scene = nullptr; // temporary save. actually don't need
		Camera* camera = nullptr; // temporary save. actually don't need

		void renderMeshToGBuffer(Mesh*);
		void renderMeshPieceToGBuffer(Mesh*, MeshGeometry*, MeshMaterial*);

		// render logic for skybox
		void renderSkybox(Skybox*);

		// render logic for each material
		void renderSolidColor(Mesh*, MeshGeometry*, ColorMaterial*);
		void renderFlatTexture(Mesh*, MeshGeometry*, TextureMaterial*);
		void renderWireframe(Mesh*, MeshGeometry*, WireframeMaterial*);

		// deferred render passes
		MeshDeferredRenderPass_Pack_SolidColor* pack_colorPass = nullptr;
		MeshDeferredRenderPass_Pack_FlatTexture* pack_texture = nullptr;
		MeshDeferredRenderPass_Pack_Wireframe* pack_wireframe = nullptr;
		MeshDeferredRenderPass_Unpack* unpack_pass = nullptr;

	};

}