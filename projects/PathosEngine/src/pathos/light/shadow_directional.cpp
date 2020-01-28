#include "shadow_directional.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/shader/shader.h"
#include "pathos/mesh/mesh.h"
#include "pathos/render/scene_render_targets.h"

#include "badger/assertion/assertion.h"
#include "glm/gtc/matrix_transform.hpp"

namespace pathos {

	DirectionalShadowMap::DirectionalShadowMap(const glm::vec3& inLightDirection) {
		lightDirection = inLightDirection;
	}

	DirectionalShadowMap::~DirectionalShadowMap() {
		CHECK(destroyed);
	}

	void DirectionalShadowMap::setLightDirection(const glm::vec3& direction) {
		lightDirection = direction;
	}

	void DirectionalShadowMap::renderShadowMap(RenderCommandList& cmdList, const Scene* scene, const Camera* camera) {
		SCOPED_DRAW_EVENT(CascadedShadowMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		static const GLfloat clear_depth_one[] = { 1.0f };

		// 1. Build projection matrices that perfectly cover each camera frustum
		if (scene->directionalLights.size() > 0) {
			setLightDirection(scene->directionalLights[0]->getDirection());
		}
		calculateBounds(*camera, sceneContext.numCascades);

		// 2. Render depth map
		cmdList.useProgram(program);
		cmdList.clipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthFunc(GL_LESS);

		std::vector<CSM_MeshBatch> meshBatches;
		std::vector<CSM_MeshBatch> wireframeBatches;
		collectMeshBatches(scene, meshBatches, wireframeBatches);

		cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (uint32 i = 0u; i < sceneContext.numCascades; ++i) {
			SCOPED_DRAW_EVENT(RenderCascade);

			cmdList.namedFramebufferTextureLayer(fbo, GL_DEPTH_ATTACHMENT, sceneContext.cascadedShadowMap, 0, i);
			cmdList.clearBufferfv(GL_DEPTH, 0, clear_depth_one);

			cmdList.viewport(0, 0, sceneContext.csmWidth, sceneContext.csmHeight);
			const glm::mat4& VP = viewProjectionMatrices[i];

			for (CSM_MeshBatch& batch : meshBatches) {
				glm::mat4 mvp = VP * batch.modelMatrix;

				cmdList.uniformMatrix4fv(uniform_depthMVP, 1, GL_FALSE, &(mvp[0][0]));
				batch.geometry->activate_position(cmdList);
				batch.geometry->activateIndexBuffer(cmdList);
				batch.geometry->drawPrimitive(cmdList);
			}

			if (wireframeBatches.size() > 0) {
				cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

				for (CSM_MeshBatch& batch : meshBatches) {
					glm::mat4 mvp = VP * batch.modelMatrix;

					cmdList.uniformMatrix4fv(uniform_depthMVP, 1, GL_FALSE, &(mvp[0][0]));
					batch.geometry->activate_position(cmdList);
					batch.geometry->activateIndexBuffer(cmdList);
					batch.geometry->drawPrimitive(cmdList);
				}

				cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	}

	void DirectionalShadowMap::initializeResources(RenderCommandList& cmdList)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_CascadedShadowMap");
		// Bind layer 0 for completeness check, but it will be reset for each layer.
		cmdList.namedFramebufferTextureLayer(fbo, GL_DEPTH_ATTACHMENT, sceneContext.cascadedShadowMap, 0, 0);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);

		GLenum fboCompleteness;
		cmdList.checkNamedFramebufferStatus(fbo, GL_FRAMEBUFFER, &fboCompleteness);

		cmdList.flushAllCommands();
		CHECK(fboCompleteness == GL_FRAMEBUFFER_COMPLETE);

		// create shadow program
		string vshader = R"(#version 430 core
layout (location = 0) in vec3 position;
layout (location = 0) uniform mat4 depthMVP;
void main() {
	gl_Position = depthMVP * vec4(position, 1.0f);
}
)";
		string fshader = R"(#version 430 core
out vec4 color;
void main() {
	color = vec4(gl_FragCoord.z, 0.0f, 0.0f, 1.0f);
}
)";

		Shader vs(GL_VERTEX_SHADER, "VS_CascadedShadowMap");
		Shader fs(GL_FRAGMENT_SHADER, "FS_CascadedShadowMap");
		vs.setSource(vshader);
		fs.setSource(fshader);

		program = pathos::createProgram(vs, fs, "Program_CascadedShadowMap");
		uniform_depthMVP = 0;
	}

	void DirectionalShadowMap::destroyResources(RenderCommandList& cmdList)
	{
		if (!destroyed) {
			cmdList.deleteFramebuffers(1, &fbo);
			cmdList.deleteProgram(program);
		}
		destroyed = true;
	}

	void DirectionalShadowMap::collectMeshBatches(const Scene* scene, std::vector<CSM_MeshBatch>& outMeshBatches, std::vector<CSM_MeshBatch>& outWireframeBatches)
	{
		outMeshBatches.clear();
		outWireframeBatches.clear();

		for (Mesh* mesh : scene->meshes) {
			if (mesh->castsShadow == false) {
				continue;
			}

			const auto geometries = mesh->getGeometries();
			const auto materials = mesh->getMaterials();
			int32 ix = 0;

			for (MeshGeometry* G : geometries) {
				const glm::mat4& modelMatrix = mesh->getTransform().getMatrix();

				if (materials[ix++]->getMaterialID() == MATERIAL_ID::WIREFRAME) {
					outWireframeBatches.push_back(CSM_MeshBatch(G, modelMatrix));
				} else {
					outMeshBatches.push_back(CSM_MeshBatch(G, modelMatrix));
				}
			}
		}
	}

	void DirectionalShadowMap::calculateBounds(const Camera& camera, uint32 numCascades)
	{
		viewProjectionMatrices.clear();

		auto calcBounds = [this](const glm::vec3* frustum) -> void {
			glm::vec3 L_forward = lightDirection;
			glm::vec3 L_up, L_right;
			pathos::calculateOrthonormalBasis(L_forward, L_up, L_right);

			glm::vec3 center(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				center += frustum[i];
			}
			center *= 0.125f;

			glm::vec3 lengths(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				glm::vec3 delta = frustum[i] - center;
				lengths.x = pathos::max(lengths.x, fabs(glm::dot(delta, L_right)));
				lengths.y = pathos::max(lengths.y, fabs(glm::dot(delta, L_up)));
				lengths.z = pathos::max(lengths.z, fabs(glm::dot(delta, L_forward)));
			}

			glm::mat4 lightView = glm::lookAt(center, center + L_forward, L_up);
			glm::mat4 projection = glm::ortho(-lengths.x, lengths.x, -lengths.y, lengths.y, -lengths.z, lengths.z);
			viewProjectionMatrices.emplace_back(projection * lightView);
		};

		std::vector<glm::vec3> frustumPlanes;
		camera.getFrustum(frustumPlanes, numCascades);
		for (uint32 i = 0u; i < numCascades; ++i) {
			calcBounds(&frustumPlanes[i * 4]);
		}
	}

}
