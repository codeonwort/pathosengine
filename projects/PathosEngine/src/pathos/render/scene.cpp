#include "scene.h"
#include "pathos/light/light.h"

namespace pathos {

	Scene::Scene() {}

	void Scene::calculateLightBuffer() {
		pointLightPositionBuffer.resize(pointLights.size() * 3);
		pointLightColorBuffer.resize(pointLights.size() * 3);
		for (auto i = 0u; i < pointLights.size(); ++i) {
			auto pos = pointLights[i]->getPosition();
			auto col = pointLights[i]->getColor();
			pointLightPositionBuffer[i * 3 + 0] = pos[0];
			pointLightPositionBuffer[i * 3 + 1] = pos[1];
			pointLightPositionBuffer[i * 3 + 2] = pos[2];
			pointLightColorBuffer[i * 3 + 0] = col[0];
			pointLightColorBuffer[i * 3 + 1] = col[1];
			pointLightColorBuffer[i * 3 + 2] = col[2];
		}

		directionalLightDirectionBuffer.resize(directionalLights.size() * 3);
		directionalLightColorBuffer.resize(directionalLights.size() * 3);
		for (auto i = 0u; i < directionalLights.size(); ++i) {
			auto dir = directionalLights[i]->getDirection();
			auto col = directionalLights[i]->getColor();
			directionalLightDirectionBuffer[i * 3 + 0] = dir[0];
			directionalLightDirectionBuffer[i * 3 + 1] = dir[1];
			directionalLightDirectionBuffer[i * 3 + 2] = dir[2];
			directionalLightColorBuffer[i * 3 + 0] = col[0];
			directionalLightColorBuffer[i * 3 + 1] = col[1];
			directionalLightColorBuffer[i * 3 + 2] = col[2];
		}
	}

	void Scene::calculateLightBufferInViewSpace(const glm::mat4& viewMatrix) {
		pointLightPositionBuffer.resize(pointLights.size() * 3);
		pointLightColorBuffer.resize(pointLights.size() * 3);
		for (auto i = 0u; i < pointLights.size(); ++i) {
			auto pos = pointLights[i]->getPosition();
			auto col = pointLights[i]->getColor();
			glm::vec4 v_pos(pos[0], pos[1], pos[2], 1.0f);
			glm::vec4 pos_viewspace = viewMatrix * v_pos;
			pointLightPositionBuffer[i * 3 + 0] = pos_viewspace.x;
			pointLightPositionBuffer[i * 3 + 1] = pos_viewspace.y;
			pointLightPositionBuffer[i * 3 + 2] = pos_viewspace.z;
			pointLightColorBuffer[i * 3 + 0] = col[0];
			pointLightColorBuffer[i * 3 + 1] = col[1];
			pointLightColorBuffer[i * 3 + 2] = col[2];
		}

		directionalLightDirectionBuffer.resize(directionalLights.size() * 3);
		directionalLightColorBuffer.resize(directionalLights.size() * 3);
		for (auto i = 0u; i < directionalLights.size(); ++i) {
			auto dir = directionalLights[i]->getDirection();
			auto col = directionalLights[i]->getColor();
			glm::vec4 v_dir(dir[0], dir[1], dir[2], 0.0f);
			glm::vec4 dir_viewspace = viewMatrix * v_dir;
			directionalLightDirectionBuffer[i * 3 + 0] = dir_viewspace.x;
			directionalLightDirectionBuffer[i * 3 + 1] = dir_viewspace.y;
			directionalLightDirectionBuffer[i * 3 + 2] = dir_viewspace.z;
			directionalLightColorBuffer[i * 3 + 0] = col[0];
			directionalLightColorBuffer[i * 3 + 1] = col[1];
			directionalLightColorBuffer[i * 3 + 2] = col[2];
		}
	}

	void Scene::add(std::initializer_list<Mesh*> newMeshes) {
		for (Mesh* mesh : newMeshes) {
			meshes.push_back(mesh);
		}
	}
}