#include "scene.h"
#include "pathos/light/light.h"

namespace pathos {

	Scene::Scene() {}

	void Scene::calculateLightBuffer() {
		pointLightPositionBuffer.resize(pointLights.size() * 3);
		pointLightColorBuffer.resize(pointLights.size() * 3);
		for (int i = 0; i < pointLights.size(); ++i) {
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
		for (int i = 0; i < directionalLights.size(); ++i) {
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

	void Scene::add(std::initializer_list<Mesh*> meshes) {
		for (Mesh* mesh : meshes) {
			this->meshes.push_back(mesh);
		}
	}
}