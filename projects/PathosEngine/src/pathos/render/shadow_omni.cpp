#include "shadow_omni.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/util/math_lib.h"

#include "badger/assertion/assertion.h"
#include "badger/types/matrix_types.h"
#include "badger/math/hit_test.h"

namespace pathos {

	struct UBO_OmniShadow {
		static constexpr GLuint BINDING_POINT = 1;

		matrix4 model;
		matrix4 viewproj;
		vector4 lightPositionAndZFar;
	};
	
	class OmniShadowVS : public ShaderStage {
	public:
		OmniShadowVS() : ShaderStage(GL_VERTEX_SHADER, "OmniShadowVS")
		{
			addDefine("VERTEX_SHADER", 1);
			setFilepath("omni_shadow_map.glsl");
		}
	};

	class OmniShadowFS : public ShaderStage {
	public:
		OmniShadowFS() : ShaderStage(GL_FRAGMENT_SHADER, "OmniShadowFS")
		{
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("omni_shadow_map.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_OmniShadow, OmniShadowVS, OmniShadowFS);

}

namespace pathos {

	// #todo-shadow: Adaptive shadow map size
	const uint32 OmniShadowPass::SHADOW_MAP_SIZE = 256;

	void OmniShadowPass::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_OmniShadowMap");

		ubo.init<UBO_OmniShadow>();
	}

	void OmniShadowPass::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void OmniShadowPass::renderShadowMaps(RenderCommandList& cmdList, const SceneProxy* scene, const Camera* camera)
	{
		SCOPED_DRAW_EVENT(OmniShadowMaps);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		static const GLfloat clear_depth_one[] = { 1.0f };

		const uint32 numTotalLights = (uint32)scene->proxyList_pointLight.size();
		uint32 numShadowCastingLights = 0;
		for (PointLightProxy* light : scene->proxyList_pointLight) {
			if (light->castsShadow) {
				numShadowCastingLights += 1;
			}
		}

		sceneContext.reallocOmniShadowMaps(cmdList, numShadowCastingLights, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		GLuint shadowMaps = sceneContext.omniShadowMaps; // cubemap array

		// Early exit
		if (numShadowCastingLights == 0) {
			return;
		}

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_OmniShadow);

		cmdList.useProgram(program.getGLName());
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthFunc(GL_LESS);
		cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);
		cmdList.viewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

		vector3 faceDirections[6] = {
			vector3(1.0f, 0.0f, 0.0f), vector3(-1.0f, 0.0f, 0.0f),
			vector3(0.0f, 1.0f, 0.0f), vector3(0.0f, -1.0f, 0.0f),
			vector3(0.0f, 0.0f, 1.0f), vector3(0.0f, 0.0f, -1.0f)
		};
		vector3 upDirections[6] = {
			vector3(0.0f, -1.0f, 0.0f), vector3(0.0f, -1.0f, 0.0f),
			vector3(0.0f, 0.0f, 1.0f), vector3(0.0f, 0.0f, -1.0f),
			vector3(0.0f, -1.0f, 0.0f), vector3(0.0f, -1.0f, 0.0f)
		};

		// #todo-frustum-culling: Stat for culling.
		int32 totalDrawcall = 0;
		int32 culledDrawcall = 0;

		for (uint32 lightIx = 0; lightIx < numTotalLights; ++lightIx) {
			SCOPED_DRAW_EVENT(OmniShadowMap);

			PointLightProxy* light = scene->proxyList_pointLight[lightIx];
			if (light->castsShadow == false) {
				continue;
			}

			constexpr float zNear = 0.1f;
			const float zFar = pathos::max(1.0f, light->attenuationRadius);
			matrix4 projection = glm::perspective(glm::radians(90.0f), 1.0f, zNear, zFar);

			PerspectiveLens shadowLens(90.0f, 1.0f, zNear, zFar);
			Camera shadowCamera(shadowLens);

			for (uint32 faceIx = 0; faceIx < 6; ++faceIx) {
				cmdList.namedFramebufferTextureLayer(fbo, GL_DEPTH_ATTACHMENT, shadowMaps, 0, lightIx * 6 + faceIx);
				cmdList.clearBufferfv(GL_DEPTH, 0, clear_depth_one);

				matrix4 lightView = glm::lookAt(light->worldPosition, light->worldPosition + faceDirections[faceIx], upDirections[faceIx]);
				matrix4 viewproj = projection * lightView;

				Frustum3D shadowFrustum;
				shadowCamera.lookAt(light->worldPosition, light->worldPosition + faceDirections[faceIx], upDirections[faceIx]);
				shadowCamera.getFrustumPlanes(shadowFrustum);

				UBO_OmniShadow uboData;
				uboData.viewproj = viewproj;
				uboData.lightPositionAndZFar = vector4(light->worldPosition, zFar);

				for (ShadowMeshProxy* batch : scene->proxyList_shadowMesh) {
					totalDrawcall++;
					bool bInFrustum = badger::hitTest::AABB_frustum(batch->worldBounds, shadowFrustum);
					if (!bInFrustum) {
						culledDrawcall++;
						continue;
					}

					uboData.model = batch->modelMatrix;
					ubo.update(cmdList, UBO_OmniShadow::BINDING_POINT, &uboData);
					
					batch->geometry->activate_position(cmdList);
					batch->geometry->activateIndexBuffer(cmdList);
					batch->geometry->drawPrimitive(cmdList);
				}
			}
		}
	}

}
