#include "render_forward.h"

// #todo: Need a kind of refactoring that was done in deferred renderer.

namespace pathos {

	static constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 8;
	static constexpr uint32_t MAX_POINT_LIGHTS = 16;
	struct UBO_PerFrame {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat3 view3x3; float __pad_view3x3[3];        // (mat3 9-byte) + (pad 3-byte) = (12-byte in glsl mat3)
		glm::mat4 viewProj;
		glm::vec4 zRange;                                 // (near, far, ?, ?)
		glm::mat4 sunViewProj[4];
		glm::vec3 eyeDirection; float __pad0;
		glm::vec3 eyePosition; uint32_t numDirLights;
		glm::vec4 dirLightDirs[MAX_DIRECTIONAL_LIGHTS];   // w components are not used
		glm::vec4 dirLightColors[MAX_DIRECTIONAL_LIGHTS]; // w components are not used
		uint32_t numPointLights; glm::vec3 __pad1;
		glm::vec4 pointLightPos[MAX_POINT_LIGHTS];        // w components are not used
		glm::vec4 pointLightColors[MAX_POINT_LIGHTS];     // w components are not used
	};

	ForwardRenderer::ForwardRenderer() {
		createShaders();
	}
	ForwardRenderer::~ForwardRenderer() {
		destroyShaders();
	}

	void ForwardRenderer::createShaders() {
		shadowMap = new ShadowMap(MAX_DIRECTIONAL_LIGHTS);
		omniShadow = new OmnidirectionalShadow(MAX_POINT_LIGHTS);

		colorPass             = new SolidColorPass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		texturePass           = new FlatTexturePass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		bumpTexturePass       = new BumpTexturePass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		wireframePass         = new WireframePass;
		shadowTexturePass     = new ShadowTexturePass;
		cubeEnvMapPass        = new CubeEnvMapPass;
		shadowCubeTexturePass = new ShadowCubeTexturePass;
		alphaOnlyTexturePass  = new AlphaOnlyTexturePass;

		colorPass->setShadowMapping(shadowMap);
		texturePass->setShadowMapping(shadowMap);
		bumpTexturePass->setShadowMapping(shadowMap);

		colorPass->setOmnidirectionalShadow(omniShadow);
		texturePass->setOmnidirectionalShadow(omniShadow);
		bumpTexturePass->setOmnidirectionalShadow(omniShadow);
	}
	void ForwardRenderer::destroyShaders() {
#define release(x) if(x) { delete x; }
		release(shadowMap);
		release(omniShadow);

		release(colorPass);
		release(texturePass);
		release(bumpTexturePass);
		release(shadowTexturePass);
		release(wireframePass);
		release(cubeEnvMapPass);
		release(shadowCubeTexturePass);
		release(alphaOnlyTexturePass);
#undef release
	}

	void ForwardRenderer::render(RenderCommandList& cmdList, Scene* inScene, Camera* inCamera) {
		scene = inScene;
		camera = inCamera;

#if 0
		// DEBUG: assertion
		for (Mesh* mesh : scene->meshes) {
			Geometries geoms = mesh->getGeometries();
			Materials materials = mesh->getMaterials();
			size_t len = geoms.size();
			assert(geoms.size() == materials.size());
			for (auto i = 0u; i < len; ++i) {
				assert(geoms[i] != nullptr && materials[i] != nullptr);
			}
		}
#endif

		// #todo-renderer: Resize render targets if window size had been changed

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		scene->calculateLightBuffer();

		// ready shadow before rendering any object
		{
			SCOPED_DRAW_EVENT(ShadowMap);

			shadowMap->clearLightDepths(static_cast<uint32_t>(scene->numDirectionalLights()));
			omniShadow->clearLightDepths(static_cast<uint32_t>(scene->numPointLights()));
			for (Mesh* mesh : scene->meshes) {
				if (mesh->visible == false) continue;
				renderLightDepth(mesh);
			}
		}

		if (scene->sky != nullptr) {
			scene->sky->render(scene, camera);
		}

		{
			SCOPED_DRAW_EVENT(BasePass);

			// #todo-occlusion: occluder or BSP tree
			for (Mesh* mesh : scene->meshes) {
				if (mesh->visible == false) continue;
				render(mesh);
			}
		}

		scene = nullptr;
		camera = nullptr;
	}

	void ForwardRenderer::renderLightDepth(Mesh* mesh) {
		const glm::mat4& modelTransform = mesh->getTransform().getMatrix();
		Geometries geoms = mesh->getGeometries();
		size_t len = geoms.size();

		if (mesh->doubleSided) glDisable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CW);

		// shadow mapping for directional light
		for (auto i = 0u; i < len; ++i) {
			for (auto light = 0u; light < scene->numDirectionalLights(); ++light) {
				shadowMap->renderLightDepth(light, scene->directionalLights[light], geoms[i], modelTransform);
			}
		}

		// omnidirectional shadow for point light
		for (auto i = 0u; i < len; ++i) {
			for (auto light = 0u; light < scene->numPointLights(); ++light) {
				omniShadow->renderLightDepth(light, scene->pointLights[light], geoms[i], modelTransform);
			}
		}

		if (mesh->doubleSided) glEnable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CCW);
	}

	void ForwardRenderer::render(Mesh* mesh) {
		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		size_t len = geoms.size();

		if (mesh->doubleSided) glDisable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CW);

		for (auto i = 0u; i < len; i++) {
			auto G = geoms[i];
			auto M = materials[i];
			renderPiece(mesh, G, M);
		}

		if (mesh->doubleSided) glEnable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CCW);
	}

	void ForwardRenderer::renderPiece(Mesh* mesh, MeshGeometry* G, Material* M) {
		/*
		// fill stencil buffer for reflection
		PlaneReflection* reflection = material->getReflectionMethod();
		if (reflection != nullptr) {
		reflection->setGeometry(subGeom);
		reflection->activate(material->getVPTransform() * modelMatrix);
		reflection->renderStencil();
		reflection->deactivate();
		}*/
		// run each pass
		glDepthFunc(GL_LEQUAL);

		// TODO: convert to array access
		// leave it like this until all material id are verified
		switch (M->getMaterialID()) {
		case MATERIAL_ID::SOLID_COLOR:
			renderSolidColor(mesh, G, static_cast<ColorMaterial*>(M));
			break;
		case MATERIAL_ID::FLAT_TEXTURE:
			renderFlatTexture(mesh, G, static_cast<TextureMaterial*>(M));
			break;
		case MATERIAL_ID::WIREFRAME:
			renderWireframe(mesh, G, static_cast<WireframeMaterial*>(M));
			break;
		case MATERIAL_ID::SHADOW_TEXTURE:
			renderShadowTexture(mesh, G, static_cast<ShadowTextureMaterial*>(M));
			break;
		case MATERIAL_ID::CUBE_ENV_MAP:
			renderCubeEnvMap(mesh, G, static_cast<CubeEnvMapMaterial*>(M));
			break;
		case MATERIAL_ID::BUMP_TEXTURE:
			renderBumpTexture(mesh, G, static_cast<BumpTextureMaterial*>(M));
			break;
		case MATERIAL_ID::CUBEMAP_SHADOW_TEXTURE:
			renderShadowCubeTexture(mesh, G, static_cast<ShadowCubeTextureMaterial*>(M));
			break;
		case MATERIAL_ID::ALPHA_ONLY_TEXTURE:
			renderAlphaOnlyTexture(mesh, G, static_cast<AlphaOnlyTextureMaterial*>(M));
			break;
		default:
			// no render pass exists for this material id. should not be here...
			assert(0);
			break;
		}

		glDepthFunc(GL_LESS);
	}


	//------------------------------------------------------------------------------------------------------
	// Render logic for each material type
	//------------------------------------------------------------------------------------------------------

	void ForwardRenderer::renderSolidColor(Mesh* mesh, MeshGeometry* G, ColorMaterial* M) {
		colorPass->setModelMatrix(mesh->getTransform().getMatrix());
		colorPass->render(scene, camera, G, M);
	}

	void ForwardRenderer::renderFlatTexture(Mesh* mesh, MeshGeometry* geom, TextureMaterial* material) {
		texturePass->setModelMatrix(mesh->getTransform().getMatrix());
		texturePass->render(scene, camera, geom, material);
	}

	void ForwardRenderer::renderWireframe(Mesh* mesh, MeshGeometry* geom, WireframeMaterial* material) {
		wireframePass->setModelMatrix(mesh->getTransform().getMatrix());
		wireframePass->render(scene, camera, geom, material);
	}

	void ForwardRenderer::renderShadowTexture(Mesh* mesh, MeshGeometry* geom, ShadowTextureMaterial* material) {
		shadowTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		shadowTexturePass->render(scene, camera, geom, material);
	}

	void ForwardRenderer::renderCubeEnvMap(Mesh* mesh, MeshGeometry* geom, CubeEnvMapMaterial* material) {
		cubeEnvMapPass->setModelMatrix(mesh->getTransform().getMatrix());
		cubeEnvMapPass->render(scene, camera, geom, material);
	}

	void ForwardRenderer::renderBumpTexture(Mesh* mesh, MeshGeometry* geom, BumpTextureMaterial* material) {
		bumpTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		bumpTexturePass->render(scene, camera, geom, material);
	}

	void ForwardRenderer::renderShadowCubeTexture(Mesh* mesh, MeshGeometry* geom, ShadowCubeTextureMaterial* material) {
		shadowCubeTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		shadowCubeTexturePass->render(scene, camera, geom, material);
	}

	void ForwardRenderer::renderAlphaOnlyTexture(Mesh* mesh, MeshGeometry* geom, AlphaOnlyTextureMaterial* material) {
		alphaOnlyTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		alphaOnlyTexturePass->render(scene, camera, geom, material);
	}

}