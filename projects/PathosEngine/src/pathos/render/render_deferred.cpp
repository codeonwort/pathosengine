#include "render_deferred.h"
#include "sky.h"
#include "pathos/mesh/mesh.h"
#include "pathos/console.h"
#include "pathos/util/log.h"
#include "visualize_depth.h"

#include <algorithm>

#define ASSERT_GL_NO_ERROR 0

namespace pathos {

	static constexpr uint32_t MAX_DIRECTIONAL_LIGHTS        = 8;
	static constexpr uint32_t MAX_POINT_LIGHTS              = 16;
	static constexpr uint32_t DIRECTIONAL_LIGHT_BUFFER_SIZE = MAX_DIRECTIONAL_LIGHTS * sizeof(glm::vec4);
	static constexpr uint32_t POINT_LIGHT_BUFFER_SIZE       = MAX_POINT_LIGHTS * sizeof(glm::vec4);
	struct UBO_PerFrame {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat3 view3x3; float __pad_view3x3[3]; // (mat3 9-byte) + (pad 3-byte) = (12-byte in glsl mat3)
		glm::mat4 viewProj;
		glm::vec4 screenResolution;
		glm::vec4 zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))
		glm::mat4 sunViewProj[4];
		glm::vec3 eyeDirection; float __pad0;
		glm::vec3 eyePosition; uint32_t numDirLights;
		glm::vec4 dirLightDirs[MAX_DIRECTIONAL_LIGHTS]; // w components are not used
		glm::vec4 dirLightColors[MAX_DIRECTIONAL_LIGHTS]; // w components are not used
		uint32_t numPointLights; glm::vec3 __pad1;
		glm::vec4 pointLightPos[MAX_POINT_LIGHTS]; // w components are not used
		glm::vec4 pointLightColors[MAX_POINT_LIGHTS]; // w components are not used
	};

	DeferredRenderer::DeferredRenderer(unsigned int width, unsigned int height)
		: width(width)
		, height(height)
	{
		createGBuffer();
		createShaders();
		ubo_perFrame.init<UBO_PerFrame>();
		sunShadowMap = new DirectionalShadowMap;
	}
	DeferredRenderer::~DeferredRenderer() {
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

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			LOG(LogFatal, "Cannot create a framebuffer for deferred renderer");
			assert(0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void DeferredRenderer::destroyGBuffer() {
		glDeleteTextures(4, fbo_attachment);
		glDeleteFramebuffers(1, &fbo);
	}

	void DeferredRenderer::render(Scene* inScene, Camera* inCamera) {
		scene = inScene;
		camera = inCamera;

#if ASSERT_GL_NO_ERROR
		glGetError();
#endif

		// #todo-renderer: Resize render targets if window size had been changed

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
		SCOPED_DRAW_EVENT(ClearGBuffer);

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
		SCOPED_DRAW_EVENT(PackGBuffer);

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
		SCOPED_DRAW_EVENT(UnpackGBuffer);

		unpack_pass->bindFramebuffer(useHDR);
		unpack_pass->setDrawBuffers(false);

		// actually not an unpack work, but rendering order is here...
		if (scene->sky) {
			scene->sky->render(scene, camera);
		}

		unpack_pass->setDrawBuffers(true);
		unpack_pass->setSunDepthMap(sunShadowMap->getDepthMapTexture());
		if (useHDR) unpack_pass->renderHDR(scene, camera);
		else unpack_pass->render(scene, camera);
	}
	
	void DeferredRenderer::updateUBO(Scene* scene, Camera* camera) {
		UBO_PerFrame data;

		data.screenResolution.x = (float)width;
		data.screenResolution.y = (float)height;

		data.view        = camera->getViewMatrix();
		data.inverseView = glm::inverse(data.view);
		data.view3x3     = glm::mat3(data.view);
		data.zRange.x    = camera->getZNear();
		data.zRange.y    = camera->getZFar();
		data.zRange.z    = camera->getFovYRadians();
		data.zRange.w    = camera->getAspectRatio();
		data.viewProj    = camera->getViewProjectionMatrix();

		data.sunViewProj[0] = sunShadowMap->getViewProjection(0);
		data.sunViewProj[1] = sunShadowMap->getViewProjection(1);
		data.sunViewProj[2] = sunShadowMap->getViewProjection(2);
		data.sunViewProj[3] = sunShadowMap->getViewProjection(3);

		data.eyeDirection = glm::vec3(camera->getViewMatrix() * glm::vec4(camera->getEyeVector(), 0.0f));
		data.eyePosition  = glm::vec3(camera->getViewMatrix() * glm::vec4(camera->getPosition(), 1.0f));

		data.numDirLights = std::min(scene->numDirectionalLights(), MAX_DIRECTIONAL_LIGHTS);
		if (data.numDirLights > 0) {
			memcpy_s(&data.dirLightDirs[0], DIRECTIONAL_LIGHT_BUFFER_SIZE, scene->getDirectionalLightDirectionBuffer(), scene->getDirectionalLightBufferSize());
			memcpy_s(&data.dirLightColors[0], DIRECTIONAL_LIGHT_BUFFER_SIZE, scene->getDirectionalLightColorBuffer(), scene->getDirectionalLightBufferSize());
		}

		data.numPointLights = std::min(scene->numPointLights(), MAX_POINT_LIGHTS);
		if (data.numPointLights > 0) {
			memcpy_s(&data.pointLightPos[0], POINT_LIGHT_BUFFER_SIZE, scene->getPointLightPositionBuffer(), scene->getPointLightBufferSize());
			memcpy_s(&data.pointLightColors[0], POINT_LIGHT_BUFFER_SIZE, scene->getPointLightColorBuffer(), scene->getPointLightBufferSize());
		}

		ubo_perFrame.update(0, &data);
	}

}
