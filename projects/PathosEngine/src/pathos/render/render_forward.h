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

		virtual void render(Scene* scene, Camera*) override;

		inline ShadowMap* getShadowMap() { return shadowMap; }
		inline OmnidirectionalShadow* getOmnidirectionalShadow() { return omniShadow; }

	private:
		void createShaders();
		void destroyShaders();

		void renderLightDepth(Mesh*);
		void render(Mesh*); // render an individual object
		void renderPiece(Mesh*, MeshGeometry*, Material*); // render each piece of an object

		void renderSolidColor(Mesh*, MeshGeometry*, ColorMaterial*);
		void renderFlatTexture(Mesh*, MeshGeometry*, TextureMaterial*);
		void renderBumpTexture(Mesh*, MeshGeometry*, BumpTextureMaterial*);
		void renderShadowTexture(Mesh*, MeshGeometry*, ShadowTextureMaterial*);
		void renderWireframe(Mesh*, MeshGeometry*, WireframeMaterial*);
		void renderCubeEnvMap(Mesh*, MeshGeometry*, CubeEnvMapMaterial*);
		void renderShadowCubeTexture(Mesh*, MeshGeometry*, ShadowCubeTextureMaterial*);
		void renderAlphaOnlyTexture(Mesh*, MeshGeometry*, AlphaOnlyTextureMaterial*);

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
