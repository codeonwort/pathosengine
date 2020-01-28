#include "render_deferred.h"

#include "sky.h"
#include "god_ray.h"
#include "visualize_depth.h"
#include "forward/translucency_rendering.h"
#include "postprocessing/ssao.h"
#include "postprocessing/bloom.h"
#include "postprocessing/tone_mapping.h"
#include "postprocessing/depth_of_field.h"
#include "postprocessing/anti_aliasing_fxaa.h"
#include "pathos/mesh/mesh.h"
#include "pathos/console.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include <algorithm>


#define ASSERT_GL_NO_ERROR 0

namespace pathos {

	static ConsoleVariable<int32> cvar_enable_dof("r.dof.enable", 0, "0 = disable DoF, 1 = enable DoF"); // #todo-dof: Sometimes generates NaN in dof subsum shader. Disable for now.
	static ConsoleVariable<int32> cvar_anti_aliasing("r.antialiasing.method", 1, "0 = disable, 1 = FXAA");

	static constexpr uint32 MAX_DIRECTIONAL_LIGHTS        = 4;
	static constexpr uint32 MAX_POINT_LIGHTS              = 8;
	static constexpr uint32 DIRECTIONAL_LIGHT_BUFFER_SIZE = MAX_DIRECTIONAL_LIGHTS * sizeof(glm::vec4);
	static constexpr uint32 POINT_LIGHT_BUFFER_SIZE       = MAX_POINT_LIGHTS * sizeof(glm::vec4);
	struct UBO_PerFrame {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat3x4 view3x3; // Name is 3x3, but type should be 3x4 due to how padding works in glsl
		glm::mat4 viewProj;
		glm::vec4 projParams;
		glm::vec4 screenResolution; // (w, h, 1/w, 1/h)
		glm::vec4 zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))
		glm::mat4 sunViewProj[4];
		glm::vec3 eyeDirection; float __pad0;
		glm::vec3 eyePosition; uint32 numDirLights;
		glm::vec4 dirLightDirs[MAX_DIRECTIONAL_LIGHTS]; // w components are not used
		glm::vec4 dirLightColors[MAX_DIRECTIONAL_LIGHTS]; // w components are not used
		uint32    numPointLights; glm::vec3 __pad1;
		glm::vec4 pointLightPos[MAX_POINT_LIGHTS]; // w components are not used
		glm::vec4 pointLightColors[MAX_POINT_LIGHTS]; // w components are not used
	};
	static constexpr GLuint SCENE_UNIFORM_BINDING_INDEX = 0;

	DeferredRenderer::DeferredRenderer(uint32 width, uint32 height)
		: sceneWidth(width)
		, sceneHeight(height)
		, antiAliasing(EAntiAliasingMethod::FXAA)
	{
		CHECK(width > 0 && height > 0);

		sceneRenderTargets.useGBuffer = true;

		createShaders();
		ubo_perFrame.init<UBO_PerFrame>();

		sunShadowMap = std::make_unique<DirectionalShadowMap>();
		godRay = std::make_unique<GodRay>();
		ssao = std::make_unique<SSAO>();
		bloomPass = std::make_unique<BloomPass>();
		toneMapping = std::make_unique<ToneMapping>();
		fxaa = std::make_unique<FXAA>();
		depthOfField = std::make_unique<DepthOfField>();
	}

	DeferredRenderer::~DeferredRenderer() {
		CHECK(destroyed);
	}

	void DeferredRenderer::initializeResources(RenderCommandList& cmdList) {
		sceneRenderTargets.reallocSceneTextures(cmdList, sceneWidth, sceneHeight);
		cmdList.flushAllCommands();
		cmdList.sceneRenderTargets = &sceneRenderTargets;

		sunShadowMap->initializeResources(cmdList);
		unpack_pass->initializeResources(cmdList);
		translucency_pass->initializeResources(cmdList);

		fullscreenQuad = std::make_unique<PlaneGeometry>(2.0f, 2.0f);

		godRay->initializeResources(cmdList);
		ssao->initializeResources(cmdList);
		bloomPass->initializeResources(cmdList);
		toneMapping->initializeResources(cmdList);
		fxaa->initializeResources(cmdList);
		depthOfField->initializeResources(cmdList);
	}

	void DeferredRenderer::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			destroyShaders();
			destroySceneRenderTargets(cmdList);

			sunShadowMap->destroyResources(cmdList);
			unpack_pass->destroyResources(cmdList);
			translucency_pass->releaseResources(cmdList);

			godRay->releaseResources(cmdList);
			ssao->releaseResources(cmdList);
			bloomPass->releaseResources(cmdList);
			toneMapping->releaseResources(cmdList);
			fxaa->releaseResources(cmdList);
			depthOfField->releaseResources(cmdList);

			cmdList.flushAllCommands();
			fullscreenQuad->dispose();
		}
		destroyed = true;
	}

	void DeferredRenderer::createShaders() {
		for (uint8 i = 0; i < (uint8)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
			pack_passes[i] = nullptr;
		}
		pack_passes[(uint8)MATERIAL_ID::SOLID_COLOR]  = new MeshDeferredRenderPass_Pack_SolidColor;
		pack_passes[(uint8)MATERIAL_ID::FLAT_TEXTURE] = new MeshDeferredRenderPass_Pack_FlatTexture;
		pack_passes[(uint8)MATERIAL_ID::WIREFRAME]    = new MeshDeferredRenderPass_Pack_Wireframe;
		pack_passes[(uint8)MATERIAL_ID::BUMP_TEXTURE] = new MeshDeferredRenderPass_Pack_BumpTexture;
		pack_passes[(uint8)MATERIAL_ID::PBR_TEXTURE]  = new MeshDeferredRenderPass_Pack_PBR;

		unpack_pass = new MeshDeferredRenderPass_Unpack;

		translucency_pass = std::make_unique<TranslucencyRendering>();

		visualizeDepth = new VisualizeDepth;
	}

	void DeferredRenderer::destroyShaders() {
		for (uint8 i = 0; i < (uint8)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
			if (pack_passes[i]) {
				delete pack_passes[i];
			}
		}
		delete unpack_pass;
	}

	void DeferredRenderer::reallocateSceneRenderTargets(RenderCommandList& cmdList) {
		sceneRenderTargets.reallocSceneTextures(cmdList, sceneWidth, sceneHeight);

		if (gbufferFBO == 0) {
			GLenum gbuffer_draw_buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			cmdList.createFramebuffers(1, &gbufferFBO);
			cmdList.objectLabel(GL_FRAMEBUFFER, gbufferFBO, -1, "FBO_gbuffer");
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT0, sceneRenderTargets.gbufferA, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT1, sceneRenderTargets.gbufferB, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT2, sceneRenderTargets.gbufferC, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_DEPTH_ATTACHMENT, sceneRenderTargets.sceneDepth, 0);
			cmdList.namedFramebufferDrawBuffers(gbufferFBO, 3, gbuffer_draw_buffers);

			GLenum framebufferCompleteness = 0;
			cmdList.checkNamedFramebufferStatus(gbufferFBO, GL_FRAMEBUFFER, &framebufferCompleteness);
			// #todo-cmd-list: Define a render command that checks framebuffer completeness rather than flushing here
			cmdList.flushAllCommands();
			CHECK(framebufferCompleteness == GL_FRAMEBUFFER_COMPLETE);
		}
	}

	void DeferredRenderer::destroySceneRenderTargets(RenderCommandList& cmdList) {
		sceneRenderTargets.freeSceneTextures(cmdList);
		cmdList.deleteFramebuffers(1, &gbufferFBO);
	}

	void DeferredRenderer::render(RenderCommandList& cmdList, Scene* inScene, Camera* inCamera) {
		scene = inScene;
		camera = inCamera;

#if ASSERT_GL_NO_ERROR
		glGetError();
#endif

		cmdList.sceneRenderTargets = &sceneRenderTargets;
		reallocateSceneRenderTargets(cmdList);

		collectRenderItems();

		// Reverse-Z
		cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

		// #todo-deprecated: No need of this. Just finish scene rendering and copy sceneDepth into sceneFinal.
		auto cvar_visualizeDepth = ConsoleVariableManager::find("r.visualize_depth");
		if (cvar_visualizeDepth->getInt() != 0) {
			visualizeDepth->render(cmdList, scene, camera);
			return;
		}

		sunShadowMap->renderShadowMap(cmdList, scene, camera);

		// ready scene for rendering
		scene->calculateLightBufferInViewSpace(camera->getViewMatrix());

		{
			SCOPED_DRAW_EVENT(ClearBackbuffer);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			cmdList.clearColor(0.0f, 0.0f, 0.0f, 0.0f);
			cmdList.clearDepth(0.0f); // zero for Reverse-Z
			cmdList.clearStencil(0);
			cmdList.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}

		// update ubo_perFrame
		updateSceneUniformBuffer(cmdList, scene, camera);

		// GodRay
		// input: static meshes
		// output: god ray texture
		godRay->renderGodRay(cmdList, scene, camera, fullscreenQuad.get());

		clearGBuffer(cmdList);

 		packGBuffer(cmdList);

		ssao->renderPostProcess(cmdList, fullscreenQuad.get());

 		unpackGBuffer(cmdList);

		// Translucency pass
		renderTranslucency(cmdList);

		//////////////////////////////////////////////////////////////////////////
		// Post-processing

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);

		// input: bright pixels in gbuffer
		// output: bloom texture
		bloomPass->renderPostProcess(cmdList, fullscreenQuad.get());

		// input: scene color, bloom, god ray
		// output: scene final
		toneMapping->renderPostProcess(cmdList, fullscreenQuad.get());

		antiAliasing = (EAntiAliasingMethod)std::max(0, std::min((int32)EAntiAliasingMethod::NumMethods, cvar_anti_aliasing.getInt()));
		switch (antiAliasing) {
			case EAntiAliasingMethod::NoAA:
				// Do nothing
				break;

			case EAntiAliasingMethod::FXAA:
				fxaa->renderPostProcess(cmdList, fullscreenQuad.get());
				break;

			default:
				break;
		}

		// input: scene final
		// output: scene final with DoF
		if (cvar_enable_dof.getInt() != 0) {
			constexpr GLuint dofRenderTarget = 0; // default framebuffer
			//dof->setInput(EPostProcessInput::PPI_0, sceneContext.toneMappingResult);
			//dof->setOutput(EPostProcessOutput::PPO_0, dofRenderTarget);
			depthOfField->renderPostProcess(cmdList, fullscreenQuad.get());
		}

#if ASSERT_GL_NO_ERROR
		assert(GL_NO_ERROR == glGetError());
#endif

		scene = nullptr;
		camera = nullptr;
	}

	void DeferredRenderer::clearGBuffer(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(ClearGBuffer);

		static const GLuint zero_ui[] = { 0, 0, 0, 0 };
		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one[] = { 1.0f };
		static const GLfloat zero_depth[] = { 0.0f };
		
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, gbufferFBO);
		cmdList.clearNamedFramebufferuiv(gbufferFBO, GL_COLOR, 0, zero_ui);
		cmdList.clearNamedFramebufferfv(gbufferFBO, GL_COLOR, 1, zero);
		cmdList.clearNamedFramebufferfv(gbufferFBO, GL_COLOR, 2, zero);
		cmdList.clearNamedFramebufferfv(gbufferFBO, GL_DEPTH, 0, zero_depth);
	}

	void DeferredRenderer::packGBuffer(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(PackGBuffer);

		// Set render state
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, gbufferFBO);
		cmdList.viewport(0, 0, sceneWidth, sceneHeight);
		cmdList.depthFunc(GL_GREATER);
		cmdList.enable(GL_DEPTH_TEST);

		const uint8 numMaterialIDs = (uint8)MATERIAL_ID::NUM_MATERIAL_IDS;
		for (uint8 i = 0; i < numMaterialIDs; ++i) {
			MeshDeferredRenderPass_Pack* const pass = pack_passes[i];

			if (pass == nullptr) {
				// #todo: implement render pass
				continue;
			}

			if (i == (uint8)MATERIAL_ID::TRANSLUCENT_SOLID_COLOR) {
				// Translucency is not rendered in gbuffer pass
				continue;
			}

			{
				SCOPED_DRAW_EVENT(BindMaterialProgram);

				pass->bindProgram(cmdList);
			}

			for (auto j = 0u; j < renderItems[i].size(); ++j) {
				const RenderItem& item = renderItems[i][j];

				// #todo-renderer: Batching by same state
				if (item.mesh->doubleSided) cmdList.disable(GL_CULL_FACE);
				if (item.mesh->renderInternal) cmdList.frontFace(GL_CW);

 				pass->setModelMatrix(item.mesh->getTransform().getMatrix());
 				pass->render(cmdList, scene, camera, item.geometry, item.material);

				// #todo-renderer: Batching by same state
				if (item.mesh->doubleSided) cmdList.enable(GL_CULL_FACE);
				if (item.mesh->renderInternal) cmdList.frontFace(GL_CCW);
			}
		}
	}

	void DeferredRenderer::unpackGBuffer(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(UnpackGBuffer);

		unpack_pass->bindFramebuffer(cmdList, useHDR);

		// actually not an unpack work, but rendering order is here...
		if (scene->sky) {
			scene->sky->render(cmdList, scene, camera);
		}

		if (useHDR) {
			unpack_pass->renderHDR(cmdList, scene, camera);
		} else {
			unpack_pass->renderLDR(cmdList, scene, camera);
		}
	}
	
	// #todo-translucency: Implement
	void DeferredRenderer::renderTranslucency(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(Translucency);

		uint8 materialID = (uint8)MATERIAL_ID::TRANSLUCENT_SOLID_COLOR;
		const std::vector<RenderItem>& meshBatches = renderItems[materialID];

		translucency_pass->renderTranslucency(cmdList, camera, meshBatches);
	}

	void DeferredRenderer::updateSceneUniformBuffer(RenderCommandList& cmdList, Scene* scene, Camera* camera) {
		UBO_PerFrame data;

		const glm::mat4& projMatrix = camera->getProjectionMatrix();

		data.screenResolution.x = (float)sceneWidth;
		data.screenResolution.y = (float)sceneHeight;
		data.screenResolution.z = 1.0f / data.screenResolution.x;
		data.screenResolution.w = 1.0f / data.screenResolution.y;

		data.view        = camera->getViewMatrix();
		data.inverseView = glm::inverse(data.view);
		data.view3x3     = glm::mat3x4(data.view);
		data.zRange.x    = camera->getZNear();
		data.zRange.y    = camera->getZFar();
		data.zRange.z    = camera->getFovYRadians();
		data.zRange.w    = camera->getAspectRatio();
		data.viewProj    = camera->getViewProjectionMatrix();
		data.projParams  = glm::vec4(1.0f / projMatrix[0][0], 1.0f / projMatrix[1][1], 0.0f, 0.0f);

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

		ubo_perFrame.update(cmdList, SCENE_UNIFORM_BINDING_INDEX, &data);
	}

	void DeferredRenderer::collectRenderItems()
	{
#if 0 // DEBUG: assert geometries and materials
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

		const uint8 numMaterialIDs = (uint8)MATERIAL_ID::NUM_MATERIAL_IDS;
		for (uint8 i = 0; i < numMaterialIDs; ++i) {
			renderItems[i].clear();
		}

		// sort by materials
		for (Mesh* mesh : scene->meshes) {
			if (mesh->visible == false) continue;

			Geometries geoms = mesh->getGeometries();
			Materials materials = mesh->getMaterials();

			for (size_t i = 0u; i < geoms.size(); ++i) {
				MeshGeometry* G = geoms[i];
				Material* M = materials[i];

				uint8 materialID = (uint8)M->getMaterialID();
				CHECK(0 <= materialID && materialID < numMaterialIDs);

				RenderItem meshBatch(mesh, G, M);
				renderItems[materialID].emplace_back(meshBatch);
			}
		}
	}

}
