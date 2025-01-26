#include "shadow_directional.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/mesh/mesh.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/util/log.h"
#include "pathos/console.h"

#include "badger/assertion/assertion.h"
#include "badger/types/matrix_types.h"
#include "badger/math/minmax.h"
#include "badger/math/vector_math.h"

namespace pathos {

	// Changing it greater than 4 will break the program. ex) see UBO_PerFrame.
	static const int32 MAX_CASCADE_COUNT = 4;

	DirectionalShadowMap::~DirectionalShadowMap() {
		CHECKF(bDestroyed, "Resource leak");
	}

	void DirectionalShadowMap::setLightDirection(const vector3& direction) {
		lightDirection = direction;
	}

	void DirectionalShadowMap::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_CascadedShadowMap");

		uboPerFrame.init<UBO_PerFrame>("UBO_PerFrame_CSM");
		uboPerObject.init<Material::UBO_PerObject>("UBO_PerObject_CSM");
	}

	void DirectionalShadowMap::releaseResources(RenderCommandList& cmdList) {
		if (!bDestroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
		}
		bDestroyed = true;
	}

	void DirectionalShadowMap::renderShadowMap(RenderCommandList& cmdList, SceneProxy* scene, const Camera* camera, const UBO_PerFrame& cachedPerFrameUBOData) {
		SCOPED_DRAW_EVENT(CascadedShadowMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		static const GLfloat clear_depth_one[] = { 1.0f };

		// #todo-light: Only deal with first directional light for now.
		const DirectionalLightProxy* lightProxy = (scene->proxyList_directionalLight.size() == 0) ? nullptr : scene->proxyList_directionalLight[0];

		sceneContext.reallocDirectionalShadowMaps(cmdList, lightProxy);
		if (lightProxy == nullptr) {
			return; // Early exit if there is no directional lights in the scene.
		}

		const uint32 numCascades = lightProxy->shadowMapCascadeCount;

		cmdList.clipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.enable(GL_DEPTH_CLAMP); // Let vertices farther than zFar to be clamped to zFar
		cmdList.depthFunc(GL_LESS);

		uint32 currentProgramHash = 0;
		uint32 currentMIID = 0xffffffff;

		cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (uint32 i = 0u; i < numCascades; ++i) {
			SCOPED_DRAW_EVENT(RenderCascade);

			cmdList.namedFramebufferTextureLayer(fbo, GL_DEPTH_ATTACHMENT, sceneContext.cascadedShadowMap, 0, i);
			cmdList.clearBufferfv(GL_DEPTH, 0, clear_depth_one);
			pathos::checkFramebufferStatus(cmdList, fbo, "DirectionalShadowMap::renderShadowMap");

			cmdList.viewport(0, 0, lightProxy->shadowMapSize, lightProxy->shadowMapSize);

			// Hack uboPerFrame.
			{
				UBO_PerFrame uboData = cachedPerFrameUBOData;
				uboData.view = viewMatrices[i];
				uboData.viewProj = viewProjectionMatrices[i];
				uboData.temporalJitter = vector4(0.0f);

				uboPerFrame.update(cmdList, UBO_PerFrame::BINDING_POINT, &uboData);
			}

			for (ShadowMeshProxy* proxy : scene->proxyList_shadowMesh) {
				Material* material = proxy->material;
				MaterialShader* materialShader = material->internal_getMaterialShader();
				
				// #todo-frustum-culling: Frustum culling for CSM
				// ...

				bool bShouldBindProgram = (currentProgramHash != materialShader->programHash);
				bool bShouldUpdateMaterialParameters = (!materialShader->bTrivialDepthOnlyPass)
					&& (bShouldBindProgram || (currentMIID != material->internal_getMaterialInstanceID()));
				bool bUseWireframeMode = material->bWireframe;
				currentProgramHash = materialShader->programHash;
				currentMIID = material->internal_getMaterialInstanceID();

				if (bShouldBindProgram) {
					SCOPED_DRAW_EVENT(BindMaterialProgram);

					uint32 programName = materialShader->program->getGLName();
					CHECK(programName != 0 && programName != 0xffffffff);
					cmdList.useProgram(programName);
				}

				// Update UBO (per object)
				{
					Material::UBO_PerObject uboData;
					uboData.modelTransform = proxy->modelMatrix;
					uboData.prevModelTransform = proxy->modelMatrix; // Doesn't matter
					uboPerObject.update(cmdList, Material::UBO_PerObject::BINDING_POINT, &uboData);
				}

				// Update UBO (material)
				if (bShouldUpdateMaterialParameters && materialShader->uboTotalBytes > 0) {
					uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(materialShader->uboTotalBytes));
					material->internal_fillUniformBuffer(uboMemory);
					materialShader->uboMaterial.update(cmdList, materialShader->uboBindingPoint, uboMemory);
				}

				// #todo-material-assembler: How to detect if binding textures is mandatory?
				// Example cases:
				// - The vertex shader uses VTF(Vertex Texture Fetch)
				// - The pixel shader uses discard
				if (bShouldUpdateMaterialParameters) {
					for (const MaterialTextureParameter& mtp : material->internal_getTextureParameters()) {
						cmdList.bindTextureUnit(mtp.binding, mtp.texture->internal_getGLName());
					}
				}

				// #todo-renderer: Batching by same state
				if (proxy->doubleSided) cmdList.disable(GL_CULL_FACE);
				if (proxy->renderInternal) cmdList.frontFace(GL_CW);
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

				if (materialShader->bTrivialDepthOnlyPass) {
					proxy->geometry->bindPositionOnlyVAO(cmdList);
				} else {
					proxy->geometry->bindFullAttributesVAO(cmdList);
				}

				proxy->geometry->drawPrimitive(cmdList);

				// #todo-renderer: Batching by same state
				if (proxy->doubleSided || bUseWireframeMode) cmdList.enable(GL_CULL_FACE);
				if (proxy->renderInternal) cmdList.frontFace(GL_CCW);
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		cmdList.disable(GL_DEPTH_CLAMP);

		// #todo-renderer: Although it reverts the UBO properly, The debug name in RenderDoc appears as "UBO_PerFrame_CSM".
		// Revert uboPerFrame.
		uboPerFrame.update(cmdList, UBO_PerFrame::BINDING_POINT, (void*)&cachedPerFrameUBOData);
	}

	float DirectionalShadowMap::getShadowMapZFar() const {
		return zFar;
	}

	void DirectionalShadowMap::updateUniformBufferData(RenderCommandList& cmdList, const SceneProxy* scene, const Camera* camera) {
		if (scene->proxyList_directionalLight.size() > 0) {
			auto lightProxy = scene->proxyList_directionalLight[0];

			zFar = lightProxy->shadowMapZFar;
			setLightDirection(lightProxy->directionWS);
			calculateBounds(*camera, lightProxy->shadowMapCascadeCount, lightProxy->shadowMapZFar);
		}
	}

	void DirectionalShadowMap::calculateBounds(const Camera& camera, uint32 numCascades, float zFar) {
		viewMatrices.clear();
		viewProjectionMatrices.clear();

		auto calcBounds = [this](const vector3* frustum) -> void {
			vector3 L_forward = lightDirection;
			vector3 L_up, L_right;
			badger::calculateOrthonormalBasis(L_forward, L_up, L_right);

			vector3 center(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				center += frustum[i];
			}
			center *= 0.125f;

			vector3 lengths(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				vector3 delta = frustum[i] - center;
				lengths.x = badger::max(lengths.x, fabs(glm::dot(delta, L_right)));
				lengths.y = badger::max(lengths.y, fabs(glm::dot(delta, L_up)));
				lengths.z = badger::max(lengths.z, fabs(glm::dot(delta, L_forward)));
			}

			matrix4 lightView = glm::lookAt(center, center + L_forward, L_up);
			matrix4 projection = glm::ortho(-lengths.x, lengths.x, -lengths.y, lengths.y, -lengths.z, lengths.z);
			viewMatrices.push_back(lightView);
			viewProjectionMatrices.emplace_back(projection * lightView);
		};

		std::vector<vector3> frustumPlanes;
		camera.getFrustumVertices(frustumPlanes, numCascades, zFar, zSlices);
		for (uint32 i = 0u; i < numCascades; ++i) {
			calcBounds(&frustumPlanes[i * 4]);
		}
	}

}
