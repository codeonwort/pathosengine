#include "scene.h"
#include "pathos/console.h"
#include "pathos/light/light.h"

namespace pathos {

	static ConsoleVariable<int> cvar_visualize_depth("r.visualize_depth", 0, "visualize the scene depth");

	Scene::Scene() {}

	void Scene::calculateLightBuffer() {
		const uint32_t numPoints = (uint32_t)pointLights.size();
		pointLightPositionBuffer.resize(numPoints);
		pointLightColorBuffer.resize(numPoints);
		for (auto i = 0u; i < numPoints; ++i) {
			pointLightPositionBuffer[i] = glm::vec4(pointLights[i]->getPosition(), 0.0f);
			pointLightColorBuffer[i]    = glm::vec4(pointLights[i]->getColor(), 0.0f);
		}

		const uint32_t numDirs = (uint32_t)directionalLights.size();
		directionalLightDirectionBuffer.resize(numDirs);
		directionalLightColorBuffer.resize(numDirs);
		for (auto i = 0u; i < numDirs; ++i) {
			directionalLightDirectionBuffer[i] = glm::vec4(directionalLights[i]->getDirection(), 0.0f);
			directionalLightColorBuffer[i]     = glm::vec4(directionalLights[i]->getColor(), 0.0f);
		}
	}

	void Scene::calculateLightBufferInViewSpace(const glm::mat4& viewMatrix) {
		const uint32_t numPoints = (uint32_t)pointLights.size();
		pointLightPositionBuffer.resize(numPoints);
		pointLightColorBuffer.resize(numPoints);
		for (auto i = 0u; i < numPoints; ++i) {
			pointLightPositionBuffer[i] = viewMatrix * glm::vec4(pointLights[i]->getPosition(), 1.0f);
			pointLightColorBuffer[i]    = glm::vec4(pointLights[i]->getColor(), 0.0f);
		}

		const uint32_t numDirs = (uint32_t)directionalLights.size();
		directionalLightDirectionBuffer.resize(numDirs);
		directionalLightColorBuffer.resize(numDirs);
		for (auto i = 0u; i < numDirs; ++i) {
			directionalLightDirectionBuffer[i] = viewMatrix * glm::vec4(directionalLights[i]->getDirection(), 0.0f);
			directionalLightColorBuffer[i]     = glm::vec4(directionalLights[i]->getColor(), 0.0f);
		}
	}

	void Scene::add(std::initializer_list<Mesh*> newMeshes) {
		for (Mesh* mesh : newMeshes) {
			meshes.push_back(mesh);
		}
	}
}
