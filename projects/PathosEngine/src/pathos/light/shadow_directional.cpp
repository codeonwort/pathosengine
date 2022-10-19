#include "shadow_directional.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/light/directional_light_component.h"
#include "pathos/shader/shader_program.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/console.h"

#include "badger/assertion/assertion.h"
#include "badger/types/matrix_types.h"

namespace pathos {

	// Light frustum could be too large if we use camera's zFar as is.
	static ConsoleVariable<float> cvar_csm_zFar("r.csm.zFar", 5000.0f, "Custom zFar for CSM");

	struct UBO_CascadedShadowMap {
		static constexpr uint32 BINDING_POINT = 1;

		matrix4 depthMVP;
	};

	class CascadedShadowMapVS : public ShaderStage {
	public:
		CascadedShadowMapVS() : ShaderStage(GL_VERTEX_SHADER, "CascadedShadowMapVS") {
			addDefine("VERTEX_SHADER", 1);
			setFilepath("cascaded_shadow_map.glsl");
		}
	};

	class CascadedShadowMapFS : public ShaderStage {
	public:
		CascadedShadowMapFS() : ShaderStage(GL_FRAGMENT_SHADER, "CascadedShadowMapFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("cascaded_shadow_map.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_CSM, CascadedShadowMapVS, CascadedShadowMapFS);

}

namespace pathos {

	DirectionalShadowMap::~DirectionalShadowMap() {
		CHECKF(destroyed, "Resource leak");
	}

	void DirectionalShadowMap::setLightDirection(const vector3& direction) {
		lightDirection = direction;
	}

	void DirectionalShadowMap::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_CascadedShadowMap");

		ubo.init<UBO_CascadedShadowMap>("UBO_CascadedShadowMap");
	}

	void DirectionalShadowMap::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
		}
		destroyed = true;
	}

	void DirectionalShadowMap::renderShadowMap(RenderCommandList& cmdList, SceneProxy* scene, const Camera* camera) {
		SCOPED_DRAW_EVENT(CascadedShadowMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		static const GLfloat clear_depth_one[] = { 1.0f };

		const ShaderProgram& program = FIND_SHADER_PROGRAM(Program_CSM);

		cmdList.useProgram(program.getGLName());
		cmdList.clipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.enable(GL_DEPTH_CLAMP); // Let vertices farther than zFar to be clamped to zFar
		cmdList.depthFunc(GL_LESS);

		cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (uint32 i = 0u; i < sceneContext.numCascades; ++i) {
			SCOPED_DRAW_EVENT(RenderCascade);

			cmdList.namedFramebufferTextureLayer(fbo, GL_DEPTH_ATTACHMENT, sceneContext.cascadedShadowMap, 0, i);
			cmdList.clearBufferfv(GL_DEPTH, 0, clear_depth_one);
			pathos::checkFramebufferStatus(cmdList, fbo, "DirectionalShadowMap::renderShadowMap");

			cmdList.viewport(0, 0, sceneContext.csmWidth, sceneContext.csmHeight);
			const matrix4& VP = viewProjectionMatrices[i];

			for (ShadowMeshProxy* batch : scene->proxyList_shadowMesh) {
				matrix4 mvp = VP * batch->modelMatrix;

				UBO_CascadedShadowMap uboData;
				uboData.depthMVP = mvp;
				ubo.update(cmdList, UBO_CascadedShadowMap::BINDING_POINT, &uboData);

				batch->geometry->activate_position(cmdList);
				batch->geometry->activateIndexBuffer(cmdList);
				batch->geometry->drawPrimitive(cmdList);
			}

			if (scene->proxyList_wireframeShadowMesh.size() > 0) {
				cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

				for (ShadowMeshProxy* batch : scene->proxyList_wireframeShadowMesh) {
					matrix4 mvp = VP * batch->modelMatrix;

					UBO_CascadedShadowMap uboData;
					uboData.depthMVP = mvp;
					ubo.update(cmdList, UBO_CascadedShadowMap::BINDING_POINT, &uboData);

					batch->geometry->activate_position(cmdList);
					batch->geometry->activateIndexBuffer(cmdList);
					batch->geometry->drawPrimitive(cmdList);
				}

				cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		cmdList.disable(GL_DEPTH_CLAMP);
	}

	void DirectionalShadowMap::updateUniformBufferData(
		RenderCommandList& cmdList,
		const SceneProxy* scene,
		const Camera* camera)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		if (scene->proxyList_directionalLight.size() > 0) {
			setLightDirection(scene->proxyList_directionalLight[0]->wsDirection);
		}
		calculateBounds(*camera, sceneContext.numCascades);
	}

	void DirectionalShadowMap::calculateBounds(const Camera& camera, uint32 numCascades)
	{
		viewProjectionMatrices.clear();

		auto calcBounds = [this](const vector3* frustum) -> void {
			vector3 L_forward = lightDirection;
			vector3 L_up, L_right;
			pathos::calculateOrthonormalBasis(L_forward, L_up, L_right);

			vector3 center(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				center += frustum[i];
			}
			center *= 0.125f;

			vector3 lengths(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				vector3 delta = frustum[i] - center;
				lengths.x = pathos::max(lengths.x, fabs(glm::dot(delta, L_right)));
				lengths.y = pathos::max(lengths.y, fabs(glm::dot(delta, L_up)));
				lengths.z = pathos::max(lengths.z, fabs(glm::dot(delta, L_forward)));
			}

			matrix4 lightView = glm::lookAt(center, center + L_forward, L_up);
			matrix4 projection = glm::ortho(-lengths.x, lengths.x, -lengths.y, lengths.y, -lengths.z, lengths.z);
			viewProjectionMatrices.emplace_back(projection * lightView);
		};

		const float zFar = std::max(1.0f, cvar_csm_zFar.getFloat());
		std::vector<vector3> frustumPlanes;
		camera.getFrustumVertices(frustumPlanes, numCascades, zFar);
		for (uint32 i = 0u; i < numCascades; ++i) {
			calcBounds(&frustumPlanes[i * 4]);
		}
	}

}
