#include "render_deferred.h"
#include "pathos/mesh/mesh.h"
#include "pathos/render/envmap.h"
#include "pathos/util/log.h"

#include <algorithm>

#define ASSERT_GL_NO_ERROR 1

namespace pathos {

	static constexpr size_t MAX_DIRECTIONAL_LIGHTS = 8;
	static constexpr size_t MAX_POINT_LIGHTS = 16;
	struct UBO_PerFrame {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat3 view3x3; float __pad_view3x3[3]; // (mat3 9-byte) + (pad 3-byte) = (12-byte in glsl mat3)
		glm::mat4 viewProj;
		glm::vec4 zRange; // (near, far, ?, ?)
		glm::mat4 sunViewProj[4];
		glm::vec3 eyeDirection; float __pad0;
		glm::vec3 eyePosition; uint32_t numDirLights;
		glm::vec4 dirLightDirs[MAX_DIRECTIONAL_LIGHTS]; // w components are not used
		glm::vec4 dirLightColors[MAX_DIRECTIONAL_LIGHTS]; // w components are not used
		uint32_t numPointLights; glm::vec3 __pad1;
		glm::vec4 pointLightPos[MAX_POINT_LIGHTS]; // w components are not used
		glm::vec4 pointLightColors[MAX_POINT_LIGHTS]; // w components are not used
	};

	DeferredRenderer::DeferredRenderer(unsigned int width, unsigned int height):width(width), height(height) {
		createGBuffer();
		createShaders();
		createUBO();
		sunShadowMap = new DirectionalShadowMap;
	}
	DeferredRenderer::~DeferredRenderer() {
		destroyUBO();
		destroyShaders();
		destroyGBuffer();
		delete sunShadowMap;
	}

	void DeferredRenderer::createShaders() {
		pack_colorPass = new MeshDeferredRenderPass_Pack_SolidColor;
		pack_texture = new MeshDeferredRenderPass_Pack_FlatTexture;
		pack_wireframe = new MeshDeferredRenderPass_Pack_Wireframe;
		pack_bumptexture = new MeshDeferredRenderPass_Pack_BumpTexture;
		pack_pbr = new MeshDeferredRenderPass_Pack_PBR;
		unpack_pass = new MeshDeferredRenderPass_Unpack(
			fbo_attachment[0], fbo_attachment[1], fbo_attachment[2],
			width, height);
	}
	void DeferredRenderer::destroyShaders() {
#define release(x) if(x) delete x
		release(pack_colorPass);
		release(pack_texture);
		release(pack_wireframe);
		release(pack_bumptexture);
		release(pack_pbr);
		release(unpack_pass);
#undef release
	}

	void DeferredRenderer::createGBuffer() {
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(4, fbo_attachment);

		glBindTexture(GL_TEXTURE_2D, fbo_attachment[0]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, fbo_attachment[1]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, fbo_attachment[2]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, fbo_attachment[3]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_attachment[0], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fbo_attachment[1], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, fbo_attachment[2], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo_attachment[3], 0);

		glDrawBuffers(3, draw_buffers);

		// check if our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std_log(LogFatal, "Cannot create a framebuffer for deferred renderer");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void DeferredRenderer::destroyGBuffer() {
		glDeleteTextures(4, fbo_attachment);
		glDeleteFramebuffers(1, &fbo);
	}

	void DeferredRenderer::createUBO() {
		glGenBuffers(1, &ubo_perFrame);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_perFrame);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_PerFrame), (void*)0, GL_DYNAMIC_DRAW);
	}

	void DeferredRenderer::destroyUBO() {
		glDeleteBuffers(1, &ubo_perFrame);
	}

	// MOST OUTER RENDERING FUNCTION
	void DeferredRenderer::render(Scene* scene_, Camera* camera_) {
		scene = scene_;
		camera = camera_;

#if ASSERT_GL_NO_ERROR
		glGetError();
#endif
		sunShadowMap->renderShadowMap(scene, camera);

		glViewport(0, 0, width, height);

		// ready scene for rendering
		scene->calculateLightBufferInViewSpace(camera->getViewMatrix());

		// update ubo_perFrame
		updateUBO(scene, camera);

		// render to fbo
		clearGBuffer();
		packGBuffer();
		unpackGBuffer();

#if ASSERT_GL_NO_ERROR
		assert(GL_NO_ERROR == glGetError());
#endif

		scene = nullptr;
		camera = nullptr;
	}

	void DeferredRenderer::clearGBuffer() {
		static const GLuint zero_ui[] = { 0, 0, 0, 0 };
		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one[] = { 1.0f };
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearBufferuiv(GL_COLOR, 0, zero_ui);
		glClearBufferfv(GL_COLOR, 1, zero);
		glClearBufferfv(GL_COLOR, 2, zero);
		glClearBufferfv(GL_DEPTH, 0, one);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void DeferredRenderer::packGBuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

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

		for (Mesh* mesh : scene->meshes) {
			if (mesh->visible == false) continue;
			renderMeshToGBuffer(mesh);
		}
	}

	void DeferredRenderer::unpackGBuffer() {
		unpack_pass->bindFramebuffer(useHDR);
		unpack_pass->setDrawBuffers(false);
		renderSkybox(scene->skybox); // actually not an unpack work, but rendering order is here...
		unpack_pass->setDrawBuffers(true);
		unpack_pass->setSunDepthMap(sunShadowMap->getDepthMapTexture());
		if (useHDR) unpack_pass->renderHDR(scene, camera);
		else unpack_pass->render(scene, camera);
	}
	
	void DeferredRenderer::renderSkybox(Skybox* sky) {
		if (!sky) return;

		glm::mat4 view = glm::mat4(glm::mat3(camera->getViewMatrix())); // view transform without transition
		glm::mat4 proj = camera->getProjectionMatrix();
		glm::mat4 transform = proj * view;
		sky->activate(transform);
		sky->render();
	}

	void DeferredRenderer::renderMeshToGBuffer(Mesh* mesh) {
		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		size_t len = geoms.size();

		if (mesh->doubleSided) glDisable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CW);

		for (auto i = 0u; i < len; i++) {
			auto G = geoms[i];
			auto M = materials[i];
			renderMeshPieceToGBuffer(mesh, G, M);
		}

		if (mesh->doubleSided) glEnable(GL_CULL_FACE);
		if (mesh->renderInternal) glFrontFace(GL_CCW);
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
		case MATERIAL_ID::BUMP_TEXTURE:
			renderBumpTexture(mesh, G, static_cast<BumpTextureMaterial*>(M));
			break;
		/*case MATERIAL_ID::SHADOW_TEXTURE:
			renderShadowTexture(mesh, G, static_cast<ShadowTextureMaterial*>(M));
			break;
		case MATERIAL_ID::CUBE_ENV_MAP:
			renderCubeEnvMap(mesh, G, static_cast<CubeEnvMapMaterial*>(M));
			break;
		case MATERIAL_ID::CUBEMAP_SHADOW_TEXTURE:
			renderShadowCubeTexture(mesh, G, static_cast<ShadowCubeTextureMaterial*>(M));
			break;*/
		case MATERIAL_ID::PBR_TEXTURE:
			renderPBR(mesh, G, static_cast<PBRTextureMaterial*>(M));
			break;
		default:
			// No render pass exists for this material id. You should not enter here.
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
	void DeferredRenderer::renderBumpTexture(Mesh* mesh, MeshGeometry* G, BumpTextureMaterial* M) {
		pack_bumptexture->setModelMatrix(mesh->getTransform().getMatrix());
		pack_bumptexture->render(scene, camera, G, M);
	}

	void DeferredRenderer::renderPBR(Mesh* mesh, MeshGeometry* G, PBRTextureMaterial* M) {
		pack_pbr->setModelMatrix(mesh->getTransform().getMatrix());
		pack_pbr->render(scene, camera, G, M);
	}

	void DeferredRenderer::updateUBO(Scene* scene, Camera* camera) {
		UBO_PerFrame data;

		data.view = camera->getViewMatrix();
		data.inverseView = glm::inverse(data.view);
		data.view3x3 = glm::mat3(data.view);
		data.zRange.x = camera->getZNear();
		data.zRange.y = camera->getZFar();
		data.viewProj = camera->getViewProjectionMatrix();

		data.sunViewProj[0] = sunShadowMap->getViewProjection(0);
		data.sunViewProj[1] = sunShadowMap->getViewProjection(1);
		data.sunViewProj[2] = sunShadowMap->getViewProjection(2);
		data.sunViewProj[3] = sunShadowMap->getViewProjection(3);

		data.eyeDirection = glm::vec3(camera->getViewMatrix() * glm::vec4(camera->getEyeVector(), 0.0f));
		data.eyePosition = glm::vec3(camera->getViewMatrix() * glm::vec4(camera->getPosition(), 1.0f));

		data.numDirLights = std::min(static_cast<uint32_t>(scene->directionalLights.size()), MAX_DIRECTIONAL_LIGHTS);
		const GLfloat* buffer = scene->getDirectionalLightDirectionBuffer();
		const GLfloat* buffer2 = scene->getDirectionalLightColorBuffer();
		for (auto i = 0u; i < data.numDirLights; ++i) {
			data.dirLightDirs[i] = glm::vec4(buffer[i * 3], buffer[i * 3 + 1], buffer[i * 3 + 2], 0.0f);
			data.dirLightColors[i] = glm::vec4(buffer2[i * 3], buffer2[i * 3 + 1], buffer2[i * 3 + 2], 1.0f);
		}

		data.numPointLights = std::min(static_cast<uint32_t>(scene->pointLights.size()), MAX_POINT_LIGHTS);
		buffer = scene->getPointLightPositionBuffer();
		buffer2 = scene->getPointLightColorBuffer();
		for (auto i = 0u; i < data.numPointLights; ++i) {
			data.pointLightPos[i] = glm::vec4(buffer[i * 3], buffer[i * 3 + 1], buffer[i * 3 + 2], 0.0f);
			data.pointLightColors[i] = glm::vec4(buffer2[i * 3], buffer2[i * 3 + 1], buffer2[i * 3 + 2], 1.0f);
		}

		glBindBuffer(GL_UNIFORM_BUFFER, ubo_perFrame);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO_PerFrame), &data);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_perFrame);
	}

}
