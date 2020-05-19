#include "render_forward.h"

// #todo-forward-rendering: Need a kind of refactoring that was done in deferred renderer.

namespace pathos {

	static constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 4;
	static constexpr uint32_t MAX_POINT_LIGHTS = 8;
	struct UBO_PerFrame {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat3 view3x3; float __pad_view3x3[3];        // (mat3 9-byte) + (pad 3-byte) = (12-byte in glsl mat3) // #todo-forward: This is wrong. Use glm::mat3x4
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

	void ForwardRenderer::initializeResources(RenderCommandList& cmdList)
	{
		// #todo-forward
	}

	void ForwardRenderer::releaseResources(RenderCommandList& cmdList)
	{
		// #todo-forward
	}

	void ForwardRenderer::createShaders() {
		shadowMap = new ShadowMap(MAX_DIRECTIONAL_LIGHTS);
		omniShadow = new OmnidirectionalShadow(MAX_POINT_LIGHTS);

		colorPass             = new SolidColorPass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		texturePass           = new FlatTexturePass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		bumpTexturePass       = new BumpTexturePass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		wireframePass         = new WireframePass;
		cubeEnvMapPass        = new CubeEnvMapPass;
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
		release(wireframePass);
		release(cubeEnvMapPass);
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

		cmdList.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// ready shadow before rendering any object
		{
			SCOPED_DRAW_EVENT(ShadowMap);

#if 0 // #todo-forward-rendering
			shadowMap->clearLightDepths(static_cast<uint32_t>(scene->numDirectionalLights()));
			omniShadow->clearLightDepths(static_cast<uint32_t>(scene->numPointLights()));
			for (Mesh* mesh : scene->meshes) {
				renderLightDepth(cmdList, mesh);
			}
#endif
		}

		if (scene->sky != nullptr) {
			scene->sky->render(cmdList, inScene, inCamera);
		}

		{
			SCOPED_DRAW_EVENT(BasePass);

#if 0 // #todo-forward-rendering
			for (Mesh* mesh : scene->meshes) {
				renderMesh(cmdList, mesh);
			}
#endif
		}

		scene = nullptr;
		camera = nullptr;
	}

	void ForwardRenderer::renderLightDepth(RenderCommandList& cmdList, Mesh* mesh) {
#if 0 // #todo-forward-rendering
		const glm::mat4& modelTransform = mesh->getTransform().getMatrix();
		Geometries geoms = mesh->getGeometries();
		size_t len = geoms.size();

		if (mesh->doubleSided) glDisable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CW);

		// shadow mapping for directional light
		for (auto i = 0u; i < len; ++i) {
			for (auto light = 0u; light < scene->numDirectionalLights(); ++light) {
				shadowMap->renderLightDepth(cmdList, light, scene->directionalLights_DEPRECATED[light], geoms[i], modelTransform);
			}
		}

		// omnidirectional shadow for point light
		for (auto i = 0u; i < len; ++i) {
			for (auto light = 0u; light < scene->numPointLights(); ++light) {
				omniShadow->renderLightDepth(cmdList, light, scene->pointLights[light], geoms[i], modelTransform);
			}
		}

		if (mesh->doubleSided) glEnable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CCW);
#endif
	}

	void ForwardRenderer::renderMesh(RenderCommandList& cmdList, Mesh* mesh) {
		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		size_t len = geoms.size();

		if (mesh->doubleSided) glDisable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CW);

		for (auto i = 0u; i < len; i++) {
			auto G = geoms[i];
			auto M = materials[i];
			renderPiece(cmdList, mesh, G, M);
		}

		if (mesh->doubleSided) glEnable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CCW);
	}

	void ForwardRenderer::renderPiece(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, Material* M) {
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
			renderSolidColor(cmdList, mesh, G, static_cast<ColorMaterial*>(M));
			break;
		case MATERIAL_ID::FLAT_TEXTURE:
			renderFlatTexture(cmdList, mesh, G, static_cast<TextureMaterial*>(M));
			break;
		case MATERIAL_ID::WIREFRAME:
			renderWireframe(cmdList, mesh, G, static_cast<WireframeMaterial*>(M));
			break;
		case MATERIAL_ID::CUBE_ENV_MAP:
			renderCubeEnvMap(cmdList, mesh, G, static_cast<CubeEnvMapMaterial*>(M));
			break;
		case MATERIAL_ID::BUMP_TEXTURE:
			renderBumpTexture(cmdList, mesh, G, static_cast<BumpTextureMaterial*>(M));
			break;
		case MATERIAL_ID::ALPHA_ONLY_TEXTURE:
			renderAlphaOnlyTexture(cmdList, mesh, G, static_cast<AlphaOnlyTextureMaterial*>(M));
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

	void ForwardRenderer::renderSolidColor(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, ColorMaterial* M) {
#if 0 // #todo-forward-rendering
		colorPass->setModelMatrix(mesh->getTransform().getMatrix());
		colorPass->renderMeshPass(cmdList, scene, camera, G, M);
#endif
	}

	void ForwardRenderer::renderFlatTexture(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, TextureMaterial* M) {
#if 0 // #todo-forward-rendering
		texturePass->setModelMatrix(mesh->getTransform().getMatrix());
		texturePass->renderMeshPass(cmdList, scene, camera, G, M);
#endif
	}

	void ForwardRenderer::renderWireframe(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, WireframeMaterial* M) {
#if 0 // #todo-forward-rendering
		wireframePass->setModelMatrix(mesh->getTransform().getMatrix());
		wireframePass->renderMeshPass(cmdList, scene, camera, G, M);
#endif
	}

	void ForwardRenderer::renderCubeEnvMap(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* G, CubeEnvMapMaterial* M) {
#if 0 // #todo-forward-rendering
		cubeEnvMapPass->setModelMatrix(mesh->getTransform().getMatrix());
		cubeEnvMapPass->renderMeshPass(cmdList, scene, camera, G, M);
#endif
	}

	void ForwardRenderer::renderBumpTexture(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* geom, BumpTextureMaterial* material) {
#if 0 // #todo-forward-rendering
		bumpTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		bumpTexturePass->renderMeshPass(cmdList, scene, camera, geom, material);
#endif
	}

	void ForwardRenderer::renderAlphaOnlyTexture(RenderCommandList& cmdList, Mesh* mesh, MeshGeometry* geom, AlphaOnlyTextureMaterial* material) {
#if 0 // #todo-forward-rendering
		alphaOnlyTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		alphaOnlyTexturePass->renderMeshPass(cmdList, scene, camera, geom, material);
#endif
	}

}
