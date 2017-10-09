#include "render_forward.h"

namespace pathos {

	MeshForwardRenderer::MeshForwardRenderer() {
		createShaders();
	}
	MeshForwardRenderer::~MeshForwardRenderer() {
		destroyShaders();
	}

	void MeshForwardRenderer::createShaders() {
		shadowMap = new ShadowMap(MAX_DIRECTIONAL_LIGHTS);
		omniShadow = new OmnidirectionalShadow(MAX_POINT_LIGHTS);

		colorPass = new SolidColorPass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		texturePass = new FlatTexturePass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		bumpTexturePass = new BumpTexturePass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		wireframePass = new WireframePass;
		shadowTexturePass = new ShadowTexturePass;
		cubeEnvMapPass = new CubeEnvMapPass;
		shadowCubeTexturePass = new ShadowCubeTexturePass;
		alphaOnlyTexturePass = new AlphaOnlyTexturePass;

		colorPass->setShadowMapping(shadowMap);
		texturePass->setShadowMapping(shadowMap);
		bumpTexturePass->setShadowMapping(shadowMap);

		colorPass->setOmnidirectionalShadow(omniShadow);
		texturePass->setOmnidirectionalShadow(omniShadow);
		bumpTexturePass->setOmnidirectionalShadow(omniShadow);
	}
	void MeshForwardRenderer::destroyShaders() {
#define release(x) if(x) delete x
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

	// MOST OUTER RENDERING FUNCTION
	void MeshForwardRenderer::render(Scene* scene, Camera* camera) {
		this->scene = scene;
		this->camera = camera;

		// DEBUG: assertion
		for (Mesh* mesh : scene->meshes) {
			Geometries geoms = mesh->getGeometries();
			Materials materials = mesh->getMaterials();
			size_t len = geoms.size();
			assert(geoms.size() == materials.size());
			for (auto i = 0; i < len; ++i) {
				assert(geoms[i] != nullptr && materials[i] != nullptr);
			}
		}

		scene->calculateLightBuffer();

		// ready shadow before rendering any object
		shadowMap->clearLightDepths(scene->numDirectionalLights());
		omniShadow->clearLightDepths(scene->numPointLights());
		for (Mesh* mesh : scene->meshes) {
			if (mesh->getVisible() == false) continue;
			renderLightDepth(mesh);
		}

		// currently no optimization. just render all objects.
		// @TODO: occluder or BSP tree
		if (scene->skybox != nullptr) render(scene->skybox);
		for (Mesh* mesh : scene->meshes) {
			if (mesh->getVisible() == false) continue;
			render(mesh);
		}

		this->scene = nullptr;
		this->camera = nullptr;
	}

	void MeshForwardRenderer::renderLightDepth(Mesh* mesh) {
		const glm::mat4& modelTransform = mesh->getTransform().getMatrix();
		Geometries geoms = mesh->getGeometries();
		size_t len = geoms.size();

		if (mesh->getDoubleSided()) glDisable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CW);

		// shadow mapping for directional light
		for (auto i = 0; i < len; ++i) {
			for (auto light = 0; light < scene->numDirectionalLights(); ++light) {
				shadowMap->renderLightDepth(light, scene->directionalLights[light], geoms[i], modelTransform);
			}
		}

		// omnidirectional shadow for point light
		for (auto i = 0; i < len; ++i) {
			for (auto light = 0; light < scene->numPointLights(); ++light) {
				omniShadow->renderLightDepth(light, scene->pointLights[light], geoms[i], modelTransform);
			}
		}

		if (mesh->getDoubleSided()) glEnable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CCW);
	}

	void MeshForwardRenderer::render(Skybox* sky) {
		glm::mat4& view = glm::mat4(glm::mat3(camera->getViewMatrix())); // view transform without transition
		glm::mat4& proj = camera->getProjectionMatrix();
		glm::mat4& transform = proj * view;
		sky->activate(transform);
		sky->render();
	}

	void MeshForwardRenderer::render(Mesh* mesh) {
		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		size_t len = geoms.size();

		if (mesh->getDoubleSided()) glDisable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CW);

		for (auto i = 0; i < len; i++) {
			auto G = geoms[i];
			auto M = materials[i];
			renderPiece(mesh, G, M);
		}

		if (mesh->getDoubleSided()) glEnable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CCW);
	}

	void MeshForwardRenderer::renderPiece(Mesh* mesh, MeshGeometry* G, MeshMaterial* M) {
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

		// TODO: convert it to array access
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

		// draw reflection target
		/*if (reflection != nullptr && (reflection->getTargets()).size() > 0) {
		auto targets = reflection->getTargets();
		glFrontFace(GL_CW);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		// impossible - target's material passes override the blend setting!
		//glEnable(GL_BLEND);
		//glBlendColor(0, 0, 0, 0.2);
		//glBlendFunc(a, b);
		for (auto i = 0; i < targets.size(); i++) {
		if (reflection->positiveSide(modelMatrix, i)) {
		glm::mat4 reflectionMatrix = reflection->getReflection(modelMatrix, i);
		renderMesh(targets[i], reflectionMatrix, material->getVPTransform(), material->getEyeVector());
		}
		}
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		//glDisable(GL_BLEND);
		}*/
		glDepthFunc(GL_LESS);
	}


	//------------------------------------------------------------------------------------------------------
	// Render logic for each material type
	//------------------------------------------------------------------------------------------------------

	void MeshForwardRenderer::renderSolidColor(Mesh* mesh, MeshGeometry* G, ColorMaterial* M) {
		colorPass->setModelMatrix(mesh->getTransform().getMatrix());
		colorPass->render(scene, camera, G, M);
	}

	void MeshForwardRenderer::renderFlatTexture(Mesh* mesh, MeshGeometry* geom, TextureMaterial* material) {
		texturePass->setModelMatrix(mesh->getTransform().getMatrix());
		texturePass->render(scene, camera, geom, material);
	}

	void MeshForwardRenderer::renderWireframe(Mesh* mesh, MeshGeometry* geom, WireframeMaterial* material) {
		wireframePass->setModelMatrix(mesh->getTransform().getMatrix());
		wireframePass->render(scene, camera, geom, material);
	}

	void MeshForwardRenderer::renderShadowTexture(Mesh* mesh, MeshGeometry* geom, ShadowTextureMaterial* material) {
		shadowTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		shadowTexturePass->render(scene, camera, geom, material);
	}

	void MeshForwardRenderer::renderCubeEnvMap(Mesh* mesh, MeshGeometry* geom, CubeEnvMapMaterial* material) {
		cubeEnvMapPass->setModelMatrix(mesh->getTransform().getMatrix());
		cubeEnvMapPass->render(scene, camera, geom, material);
	}

	void MeshForwardRenderer::renderBumpTexture(Mesh* mesh, MeshGeometry* geom, BumpTextureMaterial* material) {
		bumpTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		bumpTexturePass->render(scene, camera, geom, material);
	}

	void MeshForwardRenderer::renderShadowCubeTexture(Mesh* mesh, MeshGeometry* geom, ShadowCubeTextureMaterial* material) {
		shadowCubeTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		shadowCubeTexturePass->render(scene, camera, geom, material);
	}

	void MeshForwardRenderer::renderAlphaOnlyTexture(Mesh* mesh, MeshGeometry* geom, AlphaOnlyTextureMaterial* material) {
		alphaOnlyTexturePass->setModelMatrix(mesh->getTransform().getMatrix());
		alphaOnlyTexturePass->render(scene, camera, geom, material);
	}

}