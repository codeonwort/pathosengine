#include "render_deferred.h"
#include "pathos/mesh/mesh.h"
#include "pathos/render/envmap.h"
#include "pathos/console.h"
#include "pathos/util/log.h"
#include "visualize_depth.h"

#include <algorithm>

#define ASSERT_GL_NO_ERROR 0

namespace pathos {

	static constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 8;
	static constexpr uint32_t MAX_POINT_LIGHTS = 16;
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
		for (int i = 0; i < (int)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
			pack_passes[i] = nullptr;
		}
		pack_passes[(int)MATERIAL_ID::SOLID_COLOR]  = new MeshDeferredRenderPass_Pack_SolidColor;
		pack_passes[(int)MATERIAL_ID::FLAT_TEXTURE] = new MeshDeferredRenderPass_Pack_FlatTexture;
		pack_passes[(int)MATERIAL_ID::WIREFRAME]    = new MeshDeferredRenderPass_Pack_Wireframe;
		pack_passes[(int)MATERIAL_ID::BUMP_TEXTURE] = new MeshDeferredRenderPass_Pack_BumpTexture;
		pack_passes[(int)MATERIAL_ID::PBR_TEXTURE]  = new MeshDeferredRenderPass_Pack_PBR;

		unpack_pass = new MeshDeferredRenderPass_Unpack(
			fbo_attachment[0], fbo_attachment[1], fbo_attachment[2],
			width, height);

		visualizeDepth = new VisualizeDepth;
	}
	void DeferredRenderer::destroyShaders() {
		for (int i = 0; i < (int)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
			if (pack_passes[i]) delete pack_passes[i];
		}
		delete unpack_pass;
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
			LOG(LogFatal, "Cannot create a framebuffer for deferred renderer");
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

	void DeferredRenderer::render(Scene* inScene, Camera* inCamera) {
		scene = inScene;
		camera = inCamera;

#if ASSERT_GL_NO_ERROR
		glGetError();
#endif

		auto cvar_visualizeDepth = ConsoleVariableManager::find("r.visualize_depth");
		if (cvar_visualizeDepth->getInt() != 0) {
			glViewport(0, 0, width, height);
			visualizeDepth->render(scene, camera);
			return;
		}

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

#if 0
		// DEBUG: assert geometries and materials
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

		int numMaterialIDs = (int)MATERIAL_ID::NUM_MATERIAL_IDS;
		for (int i = 0; i < numMaterialIDs; ++i) {
			renderItems[i].clear();
		}

		// sort by materials
		for (Mesh* mesh : scene->meshes) {
			if (mesh->visible == false) continue;

			Geometries geoms = mesh->getGeometries();
			Materials materials = mesh->getMaterials();

			for (auto i = 0u; i < geoms.size(); ++i) {
				auto G = geoms[i];
				auto M = materials[i];

				int materialID = (int)M->getMaterialID();
				assert(0 <= materialID && materialID < numMaterialIDs);

				renderItems[materialID].push_back(RenderItem(mesh, G, M));
			}
		}

		glDepthFunc(GL_LESS);
		for (int i = 0; i < numMaterialIDs; ++i) {
			auto pass = pack_passes[i];

			if (pass == nullptr) {
				// #todo: implement render pass
				continue;
			}

			pass->bindProgram();

			for (auto j = 0u; j < renderItems[i].size(); ++j) {
				const RenderItem& item = renderItems[i][j];

				if (item.mesh->doubleSided) glDisable(GL_CULL_FACE);
				if (item.mesh->renderInternal) glFrontFace(GL_CW);

 				pass->setModelMatrix(item.mesh->getTransform().getMatrix());
 				pass->render(scene, camera, item.geometry, item.material);

				if (item.mesh->doubleSided) glEnable(GL_CULL_FACE);
				if (item.mesh->renderInternal) glFrontFace(GL_CCW);
			}
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

		const GLfloat* buffer;
		const GLfloat* buffer2;

		data.numDirLights = std::min(static_cast<uint32_t>(scene->directionalLights.size()), MAX_DIRECTIONAL_LIGHTS);
		if (data.numDirLights > 0) {
			buffer = scene->getDirectionalLightDirectionBuffer();
			buffer2 = scene->getDirectionalLightColorBuffer();
			for (auto i = 0u; i < data.numDirLights; ++i) {
				data.dirLightDirs[i] = glm::vec4(buffer[i * 3], buffer[i * 3 + 1], buffer[i * 3 + 2], 0.0f);
				data.dirLightColors[i] = glm::vec4(buffer2[i * 3], buffer2[i * 3 + 1], buffer2[i * 3 + 2], 1.0f);
			}
		}

		data.numPointLights = std::min(static_cast<uint32_t>(scene->pointLights.size()), MAX_POINT_LIGHTS);
		if (data.numPointLights > 0) {
			buffer = scene->getPointLightPositionBuffer();
			buffer2 = scene->getPointLightColorBuffer();
			for (auto i = 0u; i < data.numPointLights; ++i) {
				data.pointLightPos[i] = glm::vec4(buffer[i * 3], buffer[i * 3 + 1], buffer[i * 3 + 2], 0.0f);
				data.pointLightColors[i] = glm::vec4(buffer2[i * 3], buffer2[i * 3 + 1], buffer2[i * 3 + 2], 1.0f);
			}
		}

		glNamedBufferSubData(ubo_perFrame, 0, sizeof(UBO_PerFrame), &data);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_perFrame);
	}

}
