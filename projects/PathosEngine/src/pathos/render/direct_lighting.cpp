#include "direct_lighting.h"

#include "pathos/render/scene_render_targets.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/render/image_based_lighting_baker.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/mesh/geometry.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/scene/rect_light_component.h"
#include "pathos/util/log.h"
#include "pathos/util/engine_util.h"
#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/engine_policy.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_enable_shadow("r.shadow", 1, "0 = disable shadowing, 1 = enable shadowing");

	// #note: Should match with definitions in direct_lighting.glsl.
	enum class ELightSourceType : uint32 {
		Directional = 0,
		Point = 1,
		Rect = 2
	};

	template<typename LightProxy>
	struct UBO_DirectLighting {
		static_assert(
			std::is_same<LightProxy, DirectionalLightProxy>::value
				|| std::is_same<LightProxy, PointLightProxy>::value
				|| std::is_same<LightProxy, RectLightProxy>::value,
			"Not supported LightProxy type");

		static const uint32 BINDING_SLOT = 1;

		uint32     enableShadowing;
		uint32     haveShadowMap;
		uint32     omniShadowMapIndex;
		uint32     _pad0;

		LightProxy lightParameters;
	};

	template<ELightSourceType LightSourceType>
	class DirectLightingFS : public ShaderStage {
	public:
		DirectLightingFS() : ShaderStage(GL_FRAGMENT_SHADER, "DirectLightingFS")
		{
			addDefine("LIGHT_SOURCE_TYPE", (uint32)LightSourceType);
			setFilepath("direct_lighting.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_DirectLighting_Directional, FullscreenVS, DirectLightingFS<ELightSourceType::Directional>);
	DEFINE_SHADER_PROGRAM2(Program_DirectLighting_Point, FullscreenVS, DirectLightingFS<ELightSourceType::Point>);
	DEFINE_SHADER_PROGRAM2(Program_DirectLighting_Rect, FullscreenVS, DirectLightingFS<ELightSourceType::Rect>);

	static AABB getPointLightClipSpaceBounds(const vector3& centerVS, float radius, const matrix4& proj) {
		// Get clip space bounds.
		vector3 minCS(1.0f, 1.0f, 1.0f), maxCS(-1.0f, -1.0f, -1.0f);
		for (uint32 i = 0; i < 8; ++i) {
			float signX = (i & 1)        ? 1.0f : -1.0f;
			float signY = ((i >> 1) & 1) ? 1.0f : -1.0f;
			float signZ = ((i >> 2) & 1) ? 1.0f : -1.0f;
			vector3 v = centerVS + (radius * vector3(signX, signY, signZ));
			vector4 cs = proj * vector4(v, 1.0f);
			cs /= cs.w;
			v = vector3(cs);
			minCS = glm::min(minCS, v);
			maxCS = glm::max(maxCS, v);
		}
		AABB bounds = AABB::fromMinMax(minCS, maxCS);
		return bounds;
	}

}

namespace pathos {

	DirectLightingPass::DirectLightingPass()
	{
	}

	DirectLightingPass::~DirectLightingPass() {
		CHECK(bDestroyed);
	}

	void DirectLightingPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		uboDirLight.init<UBO_DirectLighting<DirectionalLightProxy>>("UBO_DirLight");
		uboPointLight.init<UBO_DirectLighting<PointLightProxy>>("UBO_PointLight");
		uboRectLight.init<UBO_DirectLighting<RectLightProxy>>("UBO_RectLight");
	}

	void DirectLightingPass::releaseResources(RenderCommandList& cmdList) {
		if (!bDestroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
		}
		bDestroyed = true;
	}

	void DirectLightingPass::bindFramebuffer(RenderCommandList& cmdList) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		// #note-lighting: Clear sceneColor as direct lighting is first
		// and then indirect lighting comes. This may change in future.
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.clearBufferfv(GL_COLOR, 0, zero);
	}

	void DirectLightingPass::renderDirectLighting(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera)
	{
		SCOPED_DRAW_EVENT(DirectLighting);
		
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		MeshGeometry* fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		// ----------------------------------------------------------
		// Common setup
		
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		pathos::checkFramebufferStatus(cmdList, fbo, "[DirectLighting] FBO is invalid");

		// Accumulate lighting to sceneColor one by one.
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.enable(GL_BLEND);
		cmdList.blendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

		GLuint gbuffer_textures[] = { sceneContext.gbufferA, sceneContext.gbufferB, sceneContext.gbufferC };
		cmdList.bindTextures(0, _countof(gbuffer_textures), gbuffer_textures);
		cmdList.bindTextureUnit(5, sceneContext.ssaoMap);

		// ----------------------------------------------------------
		// Render lighting

		renderDirectionalLights(cmdList, scene);
		renderLocalLights(cmdList, scene);

		// ----------------------------------------------------------
		// Cleanup

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		cmdList.disable(GL_BLEND);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void DirectLightingPass::renderDirectionalLights(RenderCommandList& cmdList, SceneProxy* scene) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		MeshGeometry* fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		const auto& dirLights = scene->proxyList_directionalLight;

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		if (dirLights.size() > 0) {
			SCOPED_DRAW_EVENT(DirectionalLight);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DirectLighting_Directional);
			cmdList.useProgram(program.getGLName());

			static ConsoleVariableBase* cvarZFar = ConsoleVariableManager::get().find("r.csm.zFar");
			CHECK(cvarZFar != nullptr);

			for (size_t lightIx = 0; lightIx < dirLights.size(); ++lightIx) {
				const DirectionalLightProxy* light = dirLights[lightIx];

				cmdList.bindTextureUnit(6, sceneContext.cascadedShadowMaps[lightIx]);

				UBO_DirectLighting<DirectionalLightProxy> uboData;
				uboData.enableShadowing = (uint32)((cvar_enable_shadow.getInt() != 0) && (light->bCastShadows != 0));
				uboData.haveShadowMap   = light->bCastShadows;
				uboData.lightParameters = *light;

				uboDirLight.update(cmdList, UBO_DirectLighting<DirectionalLightProxy>::BINDING_SLOT, &uboData);

				fullscreenQuad->bindFullAttributesVAO(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);
			}
		}

		cmdList.bindTextureUnit(6, NULL);
	}

	void DirectLightingPass::renderLocalLights(RenderCommandList& cmdList, SceneProxy* scene) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		MeshGeometry* fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		cmdList.bindTextureUnit(7, sceneContext.omniShadowMaps);

		const matrix4 projMatrix = scene->camera.getProjectionMatrix();

		// Point lights
		const auto& pointLights = scene->proxyList_pointLight;
		uint32 omniShadowMapIndex = 0;
		if (pointLights.size() > 0) {
			SCOPED_DRAW_EVENT(PointLight);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DirectLighting_Point);
			cmdList.useProgram(program.getGLName());
			
			// NDC Z range is [0, 1] if Reverse-Z, otherwise [-1, 1].
			float ndcMinZ = (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) ? -1.0f : 0.0f;

			for (size_t lightIx = 0; lightIx < pointLights.size(); ++lightIx) {
				const PointLightProxy* light = pointLights[lightIx];

				AABB bounds = getPointLightClipSpaceBounds(light->viewPosition, light->attenuationRadius, projMatrix);
				// Clip space to UV.
				vector2 minUV = 0.5f + 0.5f * vector2(bounds.minBounds);
				vector2 maxUV = 0.5f + 0.5f * vector2(bounds.maxBounds);
				bool bOutZ = bounds.minBounds.z > 1.0f || bounds.maxBounds.z < ndcMinZ;
				if (bOutZ || minUV.x > 1.0f || minUV.y > 1.0f || maxUV.x < 0.0f || maxUV.y < 0.0f) {
					// Current point light is out of screen.
					continue;
				}
				minUV = glm::max(vector2(0.0f), minUV);
				maxUV = glm::min(vector2(1.0f), maxUV);
				
				// Local lights usually do not cover the entire screen, so adjust the viewport.
				int32 viewportX = (int32)std::floor(minUV.x * sceneContext.sceneWidth);
				int32 viewportY = (int32)std::floor(minUV.y * sceneContext.sceneHeight);
				cmdList.viewport(
					viewportX, viewportY,
					(int32)std::ceil((maxUV.x - minUV.x) * sceneContext.sceneWidth),
					(int32)std::ceil((maxUV.y - minUV.y) * sceneContext.sceneHeight));

				UBO_DirectLighting<PointLightProxy> uboData;
				uboData.enableShadowing = cvar_enable_shadow.getInt();
				uboData.haveShadowMap = light->castsShadow;
				if (light->castsShadow) {
					uboData.omniShadowMapIndex = omniShadowMapIndex;
					omniShadowMapIndex += 1;
				}
				uboData.lightParameters = *light;

				uboPointLight.update(cmdList, UBO_DirectLighting<PointLightProxy>::BINDING_SLOT, &uboData);

				fullscreenQuad->bindFullAttributesVAO(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);
			}
		}

		// Rect lights
		const auto& rectLights = scene->proxyList_rectLight;
		if (rectLights.size() > 0) {
			SCOPED_DRAW_EVENT(RectLight);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DirectLighting_Rect);
			cmdList.useProgram(program.getGLName());

			cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

			for (size_t lightIx = 0; lightIx < rectLights.size(); ++lightIx) {
				const RectLightProxy* light = rectLights[lightIx];

				UBO_DirectLighting<RectLightProxy> uboData;
				uboData.enableShadowing = cvar_enable_shadow.getInt();
				// #todo-light: No shadowmap for rect light yet.
				uboData.haveShadowMap = false;
				//uboData.haveShadowMap = light->castsShadow;
				uboData.lightParameters = *light;

				uboRectLight.update(cmdList, UBO_DirectLighting<RectLightProxy>::BINDING_SLOT, &uboData);

				// #todo-light: Local lights usually do not cover the entire viewport.
				// Need to adjust raster region in vertex shader.
				fullscreenQuad->bindFullAttributesVAO(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);
			}
		}

		cmdList.bindTextureUnit(7, NULL);
	}

}
