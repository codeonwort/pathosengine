#include "scene.h"
#include "pathos/console.h"
#include "pathos/light/light.h"

namespace pathos {

	static ConsoleVariable<int32_t> cvar_visualize_depth("r.visualize_depth", 0, "visualize the scene depth");

	Scene::Scene() {}

	void Scene::calculateLightBuffer() {
		const uint32 numPoints = (uint32)pointLights.size();
		pointLightBuffer.resize(numPoints);
		for (uint32 i = 0u; i < numPoints; ++i) {
			pointLightBuffer[i] = pointLights[i]->getProxy();
		}

		const uint32 numDirs = (uint32)directionalLights.size();
		directionalLightDirectionBuffer.resize(numDirs);
		directionalLightColorBuffer.resize(numDirs);
		for (uint32 i = 0u; i < numDirs; ++i) {
			directionalLightDirectionBuffer[i] = glm::vec4(directionalLights[i]->getDirection(), 0.0f);
			directionalLightColorBuffer[i]     = glm::vec4(directionalLights[i]->getColor(), 0.0f);
		}
	}

	void Scene::calculateLightBufferInViewSpace(const glm::mat4& viewMatrix) {
		const uint32 numPoints = (uint32)pointLights.size();
		pointLightBuffer.resize(numPoints);
		for (uint32 i = 0u; i < numPoints; ++i) {
			pointLightBuffer[i] = pointLights[i]->getProxy();
			pointLightBuffer[i].position = glm::vec3(viewMatrix * glm::vec4(pointLightBuffer[i].position, 1.0f));
		}

		const uint32 numDirs = (uint32)directionalLights.size();
		directionalLightDirectionBuffer.resize(numDirs);
		directionalLightColorBuffer.resize(numDirs);
		for (uint32 i = 0u; i < numDirs; ++i) {
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
