#pragma once

// Render passes (shader programs) shall be maintained here by renderer, not material.

#include "renderer.h"
#include "sky.h"
#include "forward/renderpass.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"
#include "pathos/light/shadow.h"

namespace pathos {

	// Forward renderer implementation
	class ForwardRenderer : public Renderer {

	public:
		// maximum number of lights this renderer can handle
		static constexpr unsigned int MAX_DIRECTIONAL_LIGHTS = 2;
		static constexpr unsigned int MAX_POINT_LIGHTS = 4;

	public:
		ForwardRenderer();
		virtual ~ForwardRenderer();

		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void render(RenderCommandList& cmdList, Scene* scene, Camera*) override;

		inline ShadowMap* getShadowMap() { return shadowMap; }
		inline OmnidirectionalShadow* getOmnidirectionalShadow() { return omniShadow; }

	private:
		void createShaders();
		void destroyShaders();

		void renderLightDepth(RenderCommandList& cmdList, Mesh* mesh);
		void renderMesh(RenderCommandList& cmdList, Mesh* mesh); // render an individual object
		void renderPiece(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, Material* M); // render each piece of an object

		void renderSolidColor(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, ColorMaterial* M);
		void renderFlatTexture(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, TextureMaterial* M);
		void renderBumpTexture(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, BumpTextureMaterial*);
		void renderShadowTexture(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, ShadowTextureMaterial* M);
		void renderWireframe(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, WireframeMaterial* M);
		void renderCubeEnvMap(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, CubeEnvMapMaterial* M);
		void renderShadowCubeTexture(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, ShadowCubeTextureMaterial* M);
		void renderAlphaOnlyTexture(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, AlphaOnlyTextureMaterial* M);

	private:
		// material render passes
		SolidColorPass* colorPass                    = nullptr;
		FlatTexturePass* texturePass                 = nullptr;
		BumpTexturePass* bumpTexturePass             = nullptr;
		ShadowTexturePass* shadowTexturePass         = nullptr;
		WireframePass* wireframePass                 = nullptr;
		CubeEnvMapPass* cubeEnvMapPass               = nullptr;
		ShadowCubeTexturePass* shadowCubeTexturePass = nullptr;
		AlphaOnlyTexturePass* alphaOnlyTexturePass   = nullptr;

		// shadow method
		ShadowMap* shadowMap              = nullptr; // for directional light
		OmnidirectionalShadow* omniShadow = nullptr; // for point light

		// temporary variables
		Scene* scene = nullptr;
		Camera* camera = nullptr;

		// #todo-renderer: offscreen rendertargets

	};

}
