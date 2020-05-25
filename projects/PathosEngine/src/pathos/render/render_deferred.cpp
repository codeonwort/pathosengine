#include "render_deferred.h"

#include "pathos/render/render_device.h"
#include "pathos/render/sky.h"
#include "pathos/render/god_ray.h"
#include "pathos/render/visualize_depth.h"
#include "pathos/render/render_target.h"
#include "pathos/render/forward/translucency_rendering.h"
#include "pathos/render/postprocessing/ssao.h"
#include "pathos/render/postprocessing/bloom.h"
#include "pathos/render/postprocessing/tone_mapping.h"
#include "pathos/render/postprocessing/depth_of_field.h"
#include "pathos/render/postprocessing/anti_aliasing_fxaa.h"
#include "pathos/light/directional_light_component.h"
#include "pathos/light/point_light_component.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/mesh/mesh.h"
#include "pathos/console.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/gl_debug_group.h"

#include "badger/assertion/assertion.h"

#define ASSERT_GL_NO_ERROR 0

namespace pathos {

	static ConsoleVariable<int32> cvar_enable_dof("r.dof.enable", 0, "0 = disable DoF, 1 = enable DoF"); // #todo-dof: Sometimes generates NaN in dof subsum shader. Disable for now.
	static ConsoleVariable<int32> cvar_anti_aliasing("r.antialiasing.method", 1, "0 = disable, 1 = FXAA");

	static constexpr uint32 MAX_DIRECTIONAL_LIGHTS        = 4;
	static constexpr uint32 MAX_POINT_LIGHTS              = 8;
	static constexpr uint32 DIRECTIONAL_LIGHT_BUFFER_SIZE = MAX_DIRECTIONAL_LIGHTS * sizeof(DirectionalLightProxy);
	static constexpr uint32 POINT_LIGHT_BUFFER_SIZE       = MAX_POINT_LIGHTS * sizeof(PointLightProxy);
	struct UBO_PerFrame {
		glm::mat4             view;
		glm::mat4             inverseView;
		glm::mat3x4           view3x3; // Name is 3x3, but type should be 3x4 due to how padding works in glsl
		glm::mat4             viewProj;

		glm::vec4             projParams;
		glm::vec4             screenResolution; // (w, h, 1/w, 1/h)
		glm::vec4             zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))

		glm::mat4             sunViewProj[4];

		glm::vec3             eyeDirection;
		float                 __pad0;

		glm::vec3             eyePosition;
		float                 __pad1;

		glm::vec3             ws_eyePosition;
		uint32                numDirLights;

		DirectionalLightProxy directionalLights[MAX_DIRECTIONAL_LIGHTS];

		uint32                numPointLights;
		glm::vec3             __pad2;

		PointLightProxy       pointLights[MAX_POINT_LIGHTS];
	};
	static constexpr GLuint SCENE_UNIFORM_BINDING_INDEX = 0;

	DeferredRenderer::DeferredRenderer()
		: antiAliasing(EAntiAliasingMethod::FXAA)
		, scene(nullptr)
		, camera(nullptr)
	{
		sceneRenderTargets.useGBuffer = true;
	}

	DeferredRenderer::~DeferredRenderer() {
		CHECK(destroyed);
	}

	void DeferredRenderer::initializeResources(RenderCommandList& cmdList) {
		sceneRenderTargets.reallocSceneTextures(cmdList, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight);
		cmdList.flushAllCommands();
		cmdList.sceneRenderTargets = &sceneRenderTargets;
	}

	void DeferredRenderer::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			destroySceneRenderTargets(cmdList);
		}
		destroyed = true;
	}

	void DeferredRenderer::setSceneRenderSettings(const SceneRenderSettings& settings) {
		CHECK(settings.isValid());

		sceneRenderSettings = settings;
	}

	void DeferredRenderer::reallocateSceneRenderTargets(RenderCommandList& cmdList) {
		sceneRenderTargets.reallocSceneTextures(cmdList, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight);

		if (gbufferFBO == 0) {
			GLenum gbuffer_draw_buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			cmdList.createFramebuffers(1, &gbufferFBO);
			cmdList.objectLabel(GL_FRAMEBUFFER, gbufferFBO, -1, "FBO_gbuffer");
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT0, sceneRenderTargets.gbufferA, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT1, sceneRenderTargets.gbufferB, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_COLOR_ATTACHMENT2, sceneRenderTargets.gbufferC, 0);
			cmdList.namedFramebufferTexture(gbufferFBO, GL_DEPTH_ATTACHMENT, sceneRenderTargets.sceneDepth, 0);
			cmdList.namedFramebufferDrawBuffers(gbufferFBO, 3, gbuffer_draw_buffers);

			//GLenum framebufferCompleteness = 0;
			//cmdList.checkNamedFramebufferStatus(gbufferFBO, GL_FRAMEBUFFER, &framebufferCompleteness);
			//// #todo-cmd-list: Define a render command that checks framebuffer completeness rather than flushing here
			//cmdList.flushAllCommands();
			//CHECK(framebufferCompleteness == GL_FRAMEBUFFER_COMPLETE);
		}
	}

	void DeferredRenderer::destroySceneRenderTargets(RenderCommandList& cmdList) {
		sceneRenderTargets.freeSceneTextures(cmdList);
		cmdList.deleteFramebuffers(1, &gbufferFBO);
	}

	void DeferredRenderer::render(RenderCommandList& cmdList, Scene* inScene, Camera* inCamera) {
		scene = inScene;
		camera = inCamera;

		CHECK(sceneRenderSettings.isValid());

#if ASSERT_GL_NO_ERROR
		glGetError();
#endif

		cmdList.sceneRenderTargets = &sceneRenderTargets;
		reallocateSceneRenderTargets(cmdList);

		// Reverse-Z
		cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

		// #todo-deprecated: No need of this. Just finish scene rendering and copy sceneDepth into sceneFinal.
		// #todo-debug: Broken due to Reverse-Z
		auto cvar_visualizeDepth = ConsoleVariableManager::find("r.visualize_depth");
		if (cvar_visualizeDepth && cvar_visualizeDepth->getInt() != 0) {
			visualizeDepth->render(cmdList, scene, camera);
			return;
		}

		{
			SCOPED_GPU_COUNTER(RenderCascadedShadowMap);

			sunShadowMap->renderShadowMap(cmdList, scene, camera);
		}

		// ready scene for rendering
		scene->transformLightProxyToViewSpace(camera->getViewMatrix());

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
		{
			SCOPED_GPU_COUNTER(RenderGodRay);

			godRay->renderGodRay(cmdList, scene, camera, fullscreenQuad.get());
		}

		clearGBuffer(cmdList);

 		packGBuffer(cmdList);

		{
			SCOPED_GPU_COUNTER(SSAO);

			ssao->renderPostProcess(cmdList, fullscreenQuad.get());
		}

 		unpackGBuffer(cmdList);

		// Translucency pass
		{
			SCOPED_GPU_COUNTER(Translucency);

			renderTranslucency(cmdList);
		}

		//////////////////////////////////////////////////////////////////////////
		// Post-processing
		{
			SCOPED_GPU_COUNTER(PostProcessing);

			antiAliasing = (EAntiAliasingMethod)pathos::max(0, pathos::min((int32)EAntiAliasingMethod::NumMethods, cvar_anti_aliasing.getInt()));

			const bool noAA = (antiAliasing == EAntiAliasingMethod::NoAA);
			const bool noDOF = (cvar_enable_dof.getInt() == 0);

			GLuint sceneAfterLastPP = sceneRenderTargets.sceneColor;

			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);

			// Post Process: Bloom
			{
				bloomPass->setInput(EPostProcessInput::PPI_0, sceneRenderTargets.sceneBloom);
				bloomPass->setInput(EPostProcessInput::PPI_1, sceneRenderTargets.sceneBloomTemp);
				// output is fed back to PPI_0
				bloomPass->renderPostProcess(cmdList, fullscreenQuad.get());
			}

			// Post Process: Tone Mapping
			{
				// #todo-postprocess: How to check if current PP is the last? (standard way is needed, not ad-hoc like this)
				const bool isFinalPP = (noAA && noDOF);

				toneMapping->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
				toneMapping->setInput(EPostProcessInput::PPI_1, sceneRenderTargets.sceneBloom);
				toneMapping->setInput(EPostProcessInput::PPI_2, sceneRenderTargets.godRayResult);
				toneMapping->setOutput(EPostProcessOutput::PPO_0, isFinalPP ? getFinalRenderTarget() : sceneRenderTargets.toneMappingResult);
				toneMapping->renderPostProcess(cmdList, fullscreenQuad.get());

				sceneAfterLastPP = sceneRenderTargets.toneMappingResult;
			}

			// Post Process: Anti-aliasing
			switch (antiAliasing) {
			case EAntiAliasingMethod::NoAA:
				// Do nothing
				break;

			case EAntiAliasingMethod::FXAA:
				{
					// #todo-postprocess: How to check if current PP is the last? (standard way is needed, not ad-hoc like this)
					const bool isFinalPP = noDOF;

					fxaa->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
					fxaa->setOutput(EPostProcessOutput::PPO_0, isFinalPP ? getFinalRenderTarget() : sceneRenderTargets.sceneFinal);
					fxaa->renderPostProcess(cmdList, fullscreenQuad.get());

					sceneAfterLastPP = sceneRenderTargets.sceneFinal;
				}
				break;

			default:
				break;
			}

			// Post Process: Depth of Field
			if (!noDOF) {
				const GLuint dofRenderTarget = getFinalRenderTarget();

				depthOfField->setInput(EPostProcessInput::PPI_0, sceneAfterLastPP);
				depthOfField->setOutput(EPostProcessOutput::PPO_0, dofRenderTarget);
				depthOfField->renderPostProcess(cmdList, fullscreenQuad.get());
			}

		}

#if ASSERT_GL_NO_ERROR
		assert(GL_NO_ERROR == glGetError());
#endif

		scene = nullptr;
		camera = nullptr;
	}

	void DeferredRenderer::setFinalRenderTarget(RenderTarget2D* inFinalRenderTarget) {
		finalRenderTarget = inFinalRenderTarget;
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
		cmdList.viewport(0, 0, sceneRenderSettings.sceneWidth, sceneRenderSettings.sceneHeight);
		cmdList.depthFunc(GL_GREATER);
		cmdList.enable(GL_DEPTH_TEST);

		const uint8 numMaterialIDs = (uint8)MATERIAL_ID::NUM_MATERIAL_IDS;
		for (uint8 i = 0; i < numMaterialIDs; ++i) {
			MeshDeferredRenderPass_Pack* pass = pack_passes[i];

			bool fallbackPass = false;
			if (pass == nullptr) {
				pass = pack_passes[static_cast<uint16>(MATERIAL_ID::SOLID_COLOR)];
				fallbackPass = true;
			}

			if (i == (uint8)MATERIAL_ID::TRANSLUCENT_SOLID_COLOR) {
				// Translucency is not rendered in gbuffer pass
				continue;
			}

			{
				SCOPED_DRAW_EVENT(BindMaterialProgram);

				pass->bindProgram(cmdList);
			}

			const auto& proxyList = scene->proxyList_staticMesh[i];
			for (auto j = 0u; j < proxyList.size(); ++j) {
				const StaticMeshProxy& item = *(proxyList[j]);
				Material* materialOverride = fallbackPass ? fallbackMaterial.get() : item.material;

				// #todo-renderer: Batching by same state
				if (item.doubleSided) cmdList.disable(GL_CULL_FACE);
				if (item.renderInternal) cmdList.frontFace(GL_CW);

 				pass->setModelMatrix(item.modelMatrix);
 				pass->render(cmdList, scene, camera, item.geometry, materialOverride);

				// #todo-renderer: Batching by same state
				if (item.doubleSided) cmdList.enable(GL_CULL_FACE);
				if (item.renderInternal) cmdList.frontFace(GL_CCW);
			}
		}
	}

	void DeferredRenderer::unpackGBuffer(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(UnpackGBuffer);

		unpack_pass->bindFramebuffer(cmdList);

		// actually not an unpack work, but rendering order is here...
		if (scene->sky) {
			scene->sky->render(cmdList, scene, camera);
		}

		unpack_pass->render(cmdList, scene, camera);
	}
	
	// #todo-translucency: Implement
	void DeferredRenderer::renderTranslucency(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(Translucency);

		uint8 materialID = (uint8)MATERIAL_ID::TRANSLUCENT_SOLID_COLOR;
		const auto& meshBatches = scene->proxyList_staticMesh[materialID];

		translucency_pass->renderTranslucency(cmdList, camera, meshBatches);
	}

	GLuint DeferredRenderer::getFinalRenderTarget() const {
		if (finalRenderTarget == nullptr) {
			return 0; // Default backbuffer
		}
		return finalRenderTarget->getGLName();
	}

	void DeferredRenderer::updateSceneUniformBuffer(RenderCommandList& cmdList, Scene* scene, Camera* camera) {
		UBO_PerFrame data;

		const glm::mat4& projMatrix = camera->getProjectionMatrix();

		data.screenResolution.x = (float)sceneRenderSettings.sceneWidth;
		data.screenResolution.y = (float)sceneRenderSettings.sceneHeight;
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

		data.ws_eyePosition = camera->getPosition();

		data.numDirLights = pathos::min((uint32)scene->proxyList_directionalLight.size(), MAX_DIRECTIONAL_LIGHTS);
		for (uint32 i = 0; i < data.numDirLights; ++i) {
			data.directionalLights[i] = *(scene->proxyList_directionalLight[i]);
		}

		data.numPointLights = pathos::min((uint32)scene->proxyList_pointLight.size(), MAX_POINT_LIGHTS);
		for (uint32 i = 0; i < data.numPointLights; ++i) {
			data.pointLights[i] = *(scene->proxyList_pointLight[i]);
		}

		ubo_perFrame->update(cmdList, SCENE_UNIFORM_BINDING_INDEX, &data);
	}

}

namespace pathos {
	
	std::unique_ptr<class ColorMaterial>           DeferredRenderer::fallbackMaterial;
	std::unique_ptr<class PlaneGeometry>           DeferredRenderer::fullscreenQuad;
	
	std::unique_ptr<UniformBuffer>                 DeferredRenderer::ubo_perFrame;
	
	MeshDeferredRenderPass_Pack*                   DeferredRenderer::pack_passes[static_cast<uint32>(MATERIAL_ID::NUM_MATERIAL_IDS)];
	std::unique_ptr<MeshDeferredRenderPass_Unpack> DeferredRenderer::unpack_pass;
	std::unique_ptr<class TranslucencyRendering>   DeferredRenderer::translucency_pass;

	std::unique_ptr<DirectionalShadowMap>          DeferredRenderer::sunShadowMap;
	std::unique_ptr<class VisualizeDepth>          DeferredRenderer::visualizeDepth;

	std::unique_ptr<class GodRay>                  DeferredRenderer::godRay;
	std::unique_ptr<class SSAO>                    DeferredRenderer::ssao;
	std::unique_ptr<class BloomPass>               DeferredRenderer::bloomPass;
	std::unique_ptr<class ToneMapping>             DeferredRenderer::toneMapping;
	std::unique_ptr<class FXAA>                    DeferredRenderer::fxaa;
	std::unique_ptr<class DepthOfField>            DeferredRenderer::depthOfField;

	void DeferredRenderer::internal_initGlobalResources(OpenGLDevice* renderDevice) {
		RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

		fallbackMaterial = std::make_unique<ColorMaterial>();
		fallbackMaterial->setAlbedo(1.0f, 0.4f, 0.7f);
		fallbackMaterial->setMetallic(0.0f);
		fallbackMaterial->setRoughness(0.0f);

		fullscreenQuad = std::make_unique<PlaneGeometry>(2.0f, 2.0f);

		ubo_perFrame = std::make_unique<UniformBuffer>();
		ubo_perFrame->init<UBO_PerFrame>();

		{
			for (uint8 i = 0; i < (uint8)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
				pack_passes[i] = nullptr;
			}
			pack_passes[(uint8)MATERIAL_ID::SOLID_COLOR] = new MeshDeferredRenderPass_Pack_SolidColor;
			pack_passes[(uint8)MATERIAL_ID::FLAT_TEXTURE] = new MeshDeferredRenderPass_Pack_FlatTexture;
			pack_passes[(uint8)MATERIAL_ID::WIREFRAME] = new MeshDeferredRenderPass_Pack_Wireframe;
			pack_passes[(uint8)MATERIAL_ID::BUMP_TEXTURE] = new MeshDeferredRenderPass_Pack_BumpTexture;
			pack_passes[(uint8)MATERIAL_ID::PBR_TEXTURE] = new MeshDeferredRenderPass_Pack_PBR;
			unpack_pass = std::make_unique<MeshDeferredRenderPass_Unpack>();
			translucency_pass = std::make_unique<TranslucencyRendering>();

			unpack_pass->initializeResources(cmdList);
			translucency_pass->initializeResources(cmdList);
		}

		{
			sunShadowMap = std::make_unique<DirectionalShadowMap>();
			visualizeDepth = std::make_unique<VisualizeDepth>();

			sunShadowMap->initializeResources(cmdList);
		}

		{
			godRay = std::make_unique<GodRay>();
			ssao = std::make_unique<SSAO>();
			bloomPass = std::make_unique<BloomPass>();
			toneMapping = std::make_unique<ToneMapping>();
			fxaa = std::make_unique<FXAA>();
			depthOfField = std::make_unique<DepthOfField>();

			godRay->initializeResources(cmdList);
			ssao->initializeResources(cmdList);
			bloomPass->initializeResources(cmdList);
			toneMapping->initializeResources(cmdList);
			fxaa->initializeResources(cmdList);
			depthOfField->initializeResources(cmdList);
		}

		cmdList.flushAllCommands();
	}

	// #todo-scene-capture: Oh my fucking god. Freeglut does not support callback on close window.
	// Engine::stop() will not be called thus this method will not also, but std::unique_ptr's destructor will be called,
	// which invalidates the CHECK() in PostProcess' destructor.
	void DeferredRenderer::internal_destroyGlobalResources(OpenGLDevice* renderDevice) {
		RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

		fallbackMaterial.release();
		fullscreenQuad->dispose();

		ubo_perFrame.release();

		{
			for (uint8 i = 0; i < (uint8)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
				if (pack_passes[i]) {
					delete pack_passes[i];
					pack_passes[i] = nullptr;
				}
			}
			unpack_pass->destroyResources(cmdList);
			translucency_pass->releaseResources(cmdList);
		}

		{
			sunShadowMap->destroyResources(cmdList);
			visualizeDepth.release();
		}

		{
			godRay->releaseResources(cmdList);
			ssao->releaseResources(cmdList);
			bloomPass->releaseResources(cmdList);
			toneMapping->releaseResources(cmdList);
			fxaa->releaseResources(cmdList);
			depthOfField->releaseResources(cmdList);
		}

		cmdList.flushAllCommands();
	}

	DEFINE_GLOBAL_RENDER_ROUTINE(DeferredRenderer, DeferredRenderer::internal_initGlobalResources, DeferredRenderer::internal_destroyGlobalResources);

}
