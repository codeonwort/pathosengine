#include "render_deferred.h"
#include "pathos/mesh/mesh.h"
#include "pathos/render/envmap.h"

#ifdef _DEBUG
#include <iostream>
#endif

namespace pathos {

	DeferredRenderer::DeferredRenderer(unsigned int width, unsigned int height):width(width), height(height) {
		createGBuffer();
		createShaders();
	}
	DeferredRenderer::~DeferredRenderer() {
		destroyShaders();
		destroyGBuffer();
	}

	void DeferredRenderer::createShaders() {
		pack_colorPass = new MeshDeferredRenderPass_Pack_SolidColor;
		pack_texture = new MeshDeferredRenderPass_Pack_FlatTexture;
		pack_wireframe = new MeshDeferredRenderPass_Pack_Wireframe;
		unpack_pass = new MeshDeferredRenderPass_Unpack(fbo_attachment[0], fbo_attachment[1], width, height);

		/*shadowMap = new ShadowMap(MAX_DIRECTIONAL_LIGHTS);
		omniShadow = new OmnidirectionalShadow(MAX_POINT_LIGHTS);

		colorPass = new SolidColorPass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		texturePass = new FlatTexturePass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		bumpTexturePass = new BumpTexturePass(MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS);
		wireframePass = new WireframePass;
		shadowTexturePass = new ShadowTexturePass;
		cubeEnvMapPass = new CubeEnvMapPass;
		shadowCubeTexturePass = new ShadowCubeTexturePass;

		colorPass->setShadowMapping(shadowMap);
		texturePass->setShadowMapping(shadowMap);
		bumpTexturePass->setShadowMapping(shadowMap);

		colorPass->setOmnidirectionalShadow(omniShadow);
		texturePass->setOmnidirectionalShadow(omniShadow);
		bumpTexturePass->setOmnidirectionalShadow(omniShadow);*/
	}
	void DeferredRenderer::destroyShaders() {
#define release(x) if(x) delete x
		release(pack_colorPass);
		release(pack_texture);
		release(pack_wireframe);
		release(unpack_pass);

		/*release(shadowMap);
		release(omniShadow);

		release(bumpTexturePass);
		release(shadowTexturePass);
		release(cubeEnvMapPass);
		release(shadowCubeTexturePass);*/
#undef release
	}

	void DeferredRenderer::createGBuffer() {
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(3, fbo_attachment);

		glBindTexture(GL_TEXTURE_2D, fbo_attachment[0]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, fbo_attachment[1]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, fbo_attachment[2]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_attachment[0], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fbo_attachment[1], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo_attachment[2], 0);

		glDrawBuffers(2, draw_buffers);

		// check if our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#if defined(_DEBUG)
			std::cerr << "Cannot create a framebuffer for shadow map" << std::endl;
#endif
			assert(0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void DeferredRenderer::destroyGBuffer() {
		glDeleteTextures(3, fbo_attachment);
		glDeleteFramebuffers(1, &fbo);
	}

	// MOST OUTER RENDERING FUNCTION
	void DeferredRenderer::render(Scene* scene, Camera* camera) {
		this->scene = scene;
		this->camera = camera;

		// ready scene for rendering
		scene->calculateLightBuffer();

		// render to fbo
		clearGBuffer();
		packGBuffer();
		unpackGBuffer();

		this->scene = nullptr;
		this->camera = nullptr;
	}

	void DeferredRenderer::clearGBuffer() {
		static const GLint zero_i[] = { 0, 0, 0, 0 };
		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one[] = { 1.0f };

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearBufferfv(GL_COLOR, 0, zero);
		glClearBufferiv(GL_COLOR, 0, zero_i);
		glClearBufferfv(GL_COLOR, 1, zero);
		glClearBufferfv(GL_DEPTH, 0, one);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void DeferredRenderer::packGBuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glDrawBuffers(2, draw_buffers);

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

		for (Mesh* mesh : scene->meshes) {
			renderMeshToGBuffer(mesh);
		}
	}

	void DeferredRenderer::unpackGBuffer() {
		unpack_pass->bindFramebuffer(useHDR);
		unpack_pass->setDrawBuffers(false);
		renderSkybox(scene->skybox); // actually not an unpack work, but rendering order is here...
		unpack_pass->setDrawBuffers(true);
		if (useHDR) unpack_pass->renderHDR(scene, camera);
		else unpack_pass->render(scene, camera);
	}
	
	void DeferredRenderer::renderSkybox(Skybox* sky) {
		if (!sky) return;
		glm::mat4 viewTransform = camera->getViewMatrix();
		sky->activate(viewTransform);
		sky->render();
	}

	void DeferredRenderer::renderMeshToGBuffer(Mesh* mesh) {
		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		size_t len = geoms.size();

		if (mesh->getDoubleSided()) glDisable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CW);

		for (auto i = 0; i < len; i++) {
			auto G = geoms[i];
			auto M = materials[i];
			renderMeshPieceToGBuffer(mesh, G, M);
		}

		if (mesh->getDoubleSided()) glEnable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CCW);
	}

	void DeferredRenderer::renderMeshPieceToGBuffer(Mesh* mesh, MeshGeometry* G, MeshMaterial* M) {
		glDepthFunc(GL_LESS);

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
		/*case MATERIAL_ID::SHADOW_TEXTURE:
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
			break;*/
		default:
			// no render pass exists for this material id. should not be here...
			assert(0);
			break;
		}
	}

	void DeferredRenderer::renderSolidColor(Mesh* mesh, MeshGeometry* G, ColorMaterial* M) {
		pack_colorPass->setModelMatrix(mesh->getTransform().getMatrix());
		pack_colorPass->render(scene, camera, G, M);
	}
	void DeferredRenderer::renderFlatTexture(Mesh* mesh, MeshGeometry* G, TextureMaterial* M) {
		pack_texture->setModelMatrix(mesh->getTransform().getMatrix());
		pack_texture->render(scene, camera, G, M);
	}
	void DeferredRenderer::renderWireframe(Mesh* mesh, MeshGeometry* G, WireframeMaterial* M) {
		pack_wireframe->setModelMatrix(mesh->getTransform().getMatrix());
		pack_wireframe->render(scene, camera, G, M);
	}

}