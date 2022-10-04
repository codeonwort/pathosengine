#include "direct_lighting.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/shader/shader_program.h"
#include "pathos/camera/camera.h"
#include "pathos/light/directional_light_component.h"
#include "pathos/light/point_light_component.h"
#include "pathos/light/rect_light_component.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/engine_util.h"

#include "badger/assertion/assertion.h"

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

		uint32 enableShadowing;
		uint32 haveShadowMap;
		uint32 omniShadowMapIndex;
		float csmZFar;

		LightProxy lightParameters;
	};

	class DirectLightingVS : public ShaderStage {
	public:
		DirectLightingVS() : ShaderStage(GL_VERTEX_SHADER, "DirectLightingVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
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
	DEFINE_SHADER_PROGRAM2(Program_DirectLighting_Directional, DirectLightingVS, DirectLightingFS<ELightSourceType::Directional>);
	DEFINE_SHADER_PROGRAM2(Program_DirectLighting_Point, DirectLightingVS, DirectLightingFS<ELightSourceType::Point>);
	DEFINE_SHADER_PROGRAM2(Program_DirectLighting_Rect, DirectLightingVS, DirectLightingFS<ELightSourceType::Rect>);

}

namespace pathos {

	DirectLightingPass::DirectLightingPass()
		: fbo(0xffffffff)
	{
	}

	DirectLightingPass::~DirectLightingPass() {
		CHECK(destroyed);
	}

	void DirectLightingPass::initializeResources(RenderCommandList& cmdList) {
		// fullscreen quad
		quad = new PlaneGeometry(2.0f, 2.0f);
		
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		uboDirLight.init<UBO_DirectLighting<DirectionalLightProxy>>("UBO_DirLight");
		uboPointLight.init<UBO_DirectLighting<PointLightProxy>>("UBO_PointLight");
		uboRectLight.init<UBO_DirectLighting<RectLightProxy>>("UBO_RectLight");
	}

	void DirectLightingPass::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
			quad->dispose();
			delete quad;
		}
		destroyed = true;
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

	void DirectLightingPass::renderDirectLighting(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera)
	{
		SCOPED_DRAW_EVENT(DirectLighting);
		
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		// Accumulate lighting to sceneColor one by one.
		{
			cmdList.disable(GL_DEPTH_TEST);
			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
		}
		
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		pathos::checkFramebufferStatus(cmdList, fbo, "[DirectLighting] FBO is invalid");

		GLuint gbuffer_textures[] = { sceneContext.gbufferA, sceneContext.gbufferB, sceneContext.gbufferC };
		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(5, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(6, sceneContext.cascadedShadowMap);
		cmdList.bindTextureUnit(7, sceneContext.omniShadowMaps);

		// Directional lights
		const auto& dirLights = scene->proxyList_directionalLight;
		if (dirLights.size() > 0) {
			SCOPED_DRAW_EVENT(DirectionalLight);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DirectLighting_Directional);
			cmdList.useProgram(program.getGLName());

			static ConsoleVariableBase* cvarZFar = ConsoleVariableManager::get().find("r.csm.zFar");
			CHECK(cvarZFar != nullptr);

			for (size_t lightIx = 0; lightIx < dirLights.size(); ++lightIx) {
				const DirectionalLightProxy* light = dirLights[lightIx];

				UBO_DirectLighting<DirectionalLightProxy> uboData;
				uboData.enableShadowing = cvar_enable_shadow.getInt();
				// #todo-light: Support shadow map for secondary directional lights.
				uboData.haveShadowMap = (lightIx == 0);
				uboData.csmZFar = std::max(1.0f, cvarZFar->getFloat());
				uboData.lightParameters = *light;

				uboDirLight.update(cmdList, UBO_DirectLighting<DirectionalLightProxy>::BINDING_SLOT, &uboData);

				quad->activate_position_uv(cmdList);
				quad->activateIndexBuffer(cmdList);
				quad->drawPrimitive(cmdList);
			}
		}

		// Point lights
		const auto& pointLights = scene->proxyList_pointLight;
		uint32 omniShadowMapIndex = 0;
		if (pointLights.size() > 0) {
			SCOPED_DRAW_EVENT(PointLight);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DirectLighting_Point);
			cmdList.useProgram(program.getGLName());

			for (size_t lightIx = 0; lightIx < pointLights.size(); ++lightIx) {
				const PointLightProxy* light = pointLights[lightIx];

				UBO_DirectLighting<PointLightProxy> uboData;
				uboData.enableShadowing = cvar_enable_shadow.getInt();
				uboData.haveShadowMap = light->castsShadow;
				if (light->castsShadow) {
					uboData.omniShadowMapIndex = omniShadowMapIndex;
					omniShadowMapIndex += 1;
				}
				uboData.lightParameters = *light;

				uboPointLight.update(cmdList, UBO_DirectLighting<PointLightProxy>::BINDING_SLOT, &uboData);

				// #todo-light: Local lights usually do not cover the entire viewport.
				// Need to adjust raster region in vertex shader.
				quad->activate_position_uv(cmdList);
				quad->activateIndexBuffer(cmdList);
				quad->drawPrimitive(cmdList);
			}
		}

		// Rect lights
		const auto& rectLights = scene->proxyList_rectLight;
		if (rectLights.size() > 0) {
			SCOPED_DRAW_EVENT(RectLight);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DirectLighting_Rect);
			cmdList.useProgram(program.getGLName());

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
				quad->activate_position_uv(cmdList);
				quad->activateIndexBuffer(cmdList);
				quad->drawPrimitive(cmdList);
			}
		}

		// Cleanup render states
		{
			cmdList.disable(GL_BLEND);
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}

}
