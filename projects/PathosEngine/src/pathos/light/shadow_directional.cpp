#include "shadow_directional.h"
#include "pathos/util/log.h"
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

		// 1. clear depth map
		static const GLfloat clear_depth_one[] = { 1.0f };
		cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
		cmdList.clearBufferfv(GL_DEPTH, 0, clear_depth_one);

		// 2. build projection matrix that perfectly covers camera frustum
		if (scene->directionalLights.size() > 0) {
			setLightDirection(scene->directionalLights[0]->getDirection());
		}

		auto calcBounds = [this](const glm::vec3* frustum) -> void {
			glm::vec3 sun_origin(0.0f, 0.0f, 0.0f);
			glm::vec3 sun_direction = lightDirection;
			glm::vec3 sun_up(0.0f, 1.0f, 0.0f);

			// if almost parallel, choose another random direction
			float angle = glm::dot(sun_up, sun_direction);
			if (angle >= 0.999f || angle <= -0.999f) {
				sun_up = glm::vec3(1.0f, 0.0f, 0.0f);
			}

			glm::mat4 lightView = glm::lookAt(sun_origin, sun_direction, sun_up);

			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();
			float left = std::numeric_limits<float>::max();
			float right = std::numeric_limits<float>::min();
			float top = std::numeric_limits<float>::max();
			float bottom = std::numeric_limits<float>::min();

			for (int i = 0; i < 8; ++i) {
				glm::vec3 fv(lightView * glm::vec4(frustum[i], 1.0f));
				if (fv.x < left) left = fv.x;
				if (fv.x > right) right = fv.x;
				if (fv.y < top) top = fv.y;
				if (fv.y > bottom) bottom = fv.y;
				if (fv.z < minZ) minZ = fv.z;
				if (fv.z > maxZ) maxZ = fv.z;
			}

			sun_origin += sun_direction * minZ;
			// get row of lightView
			glm::vec3 sun_right = glm::vec3(lightView[0][0], lightView[1][0], lightView[2][0]);
			sun_up = glm::vec3(lightView[0][1], lightView[1][1], lightView[2][1]);
			sun_origin -= ((left + right) / 2.0f) * sun_right;
			sun_origin -= ((top + bottom) / 2.0f) * sun_up;
			lightView = glm::lookAt(sun_origin, sun_origin + sun_direction, sun_up);

			glm::mat4 projection = glm::ortho(left, right, top, bottom, 0.0f, maxZ - minZ);
			viewProjection.emplace_back(projection * lightView);
		};

		std::vector<glm::vec3> frustum;
		camera->getFrustum(frustum, sceneContext.numCascades);
		viewProjection.clear();
		for (auto i = 0u; i < sceneContext.numCascades; ++i) {
			calcBounds(&frustum[i * 4]);
		}

		// 3. render depth map
		cmdList.useProgram(program);
		cmdList.enable(GL_DEPTH_TEST);
		
		for (auto i = 0u; i < sceneContext.numCascades; ++i) {
			cmdList.viewport(i * sceneContext.csmWidth, 0, sceneContext.csmWidth, sceneContext.csmHeight);
			const glm::mat4& VP = viewProjection[i];

			for (Mesh* mesh : scene->meshes) {
				if (mesh->castsShadow == false) {
					continue;
				}

				const auto geometries = mesh->getGeometries();
				const auto materials = mesh->getMaterials();
				int ix = 0;
				for (const auto G : geometries) {
					glm::mat4 mvp = VP * mesh->getTransform().getMatrix();
					cmdList.uniformMatrix4fv(uniform_depthMVP, 1, GL_FALSE, &(mvp[0][0]));

					bool wasWireframe = false;
					if (materials[ix++]->getMaterialID() == MATERIAL_ID::WIREFRAME) {
						cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);
						wasWireframe = true;
					}

					G->activate_position(cmdList);
					G->activateIndexBuffer(cmdList);
					G->drawPrimitive(cmdList);

					if (wasWireframe) {
						cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
					}
				}
			}
		}
	}

	void DirectionalShadowMap::initializeResources(RenderCommandList& cmdList)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		// create framebuffer object
		cmdList.createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_CascadedShadowMap");
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.cascadedShadowMap, 0);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);
		
		GLenum fboCompleteness;
		cmdList.checkNamedFramebufferStatus(fbo, GL_FRAMEBUFFER, &fboCompleteness);

		// #todo-cmd-list: fboCompleteness is a local variable, can't be used like this
		// Maybe it should be allocated and returned by the command list (something like cmdList->allocVolatileVariable<GLenum>())
		//cmdList.registerHook([](void* argument) -> void {
		//	GLenum completeness = *(GLenum*)argument;
		//	CHECK(completeness == GL_FRAMEBUFFER_COMPLETE);
		//}, &fboCompleteness);
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

}
