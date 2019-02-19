#pragma once

#include "pathos/light/light.h"
#include <vector>

namespace pathos {

	// Forward decl
	class Mesh;
	class DirectionalLight;
	class PointLight;
	class Skybox;

	// Represents a 3D scene.
	class Scene {

	protected:
		std::vector<GLfloat> pointLightPositionBuffer;
		std::vector<GLfloat> pointLightColorBuffer;
		std::vector<GLfloat> directionalLightDirectionBuffer;
		std::vector<GLfloat> directionalLightColorBuffer;

	public:
		Scene();
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;

		Skybox* skybox = nullptr;
		Mesh* godRaySource = nullptr;
		std::vector<Mesh*> meshes;

		std::vector<DirectionalLight*> directionalLights;
		std::vector<PointLight*> pointLights;

		inline void add(Mesh* mesh) { meshes.push_back(mesh); }
		void add(std::initializer_list<Mesh*> meshes);
		
		inline void add(DirectionalLight* light) { directionalLights.push_back(light); }
		inline void add(PointLight* light) { pointLights.push_back(light); }
		inline size_t numDirectionalLights() { return directionalLights.size(); }
		inline size_t numPointLights() { return pointLights.size(); }

		void calculateLightBuffer(); // in world space
		void calculateLightBufferInViewSpace(const glm::mat4& viewMatrix);
		inline const GLfloat* getPointLightPositionBuffer() { return &pointLightPositionBuffer[0]; }
		inline const GLfloat* getPointLightColorBuffer() { return &pointLightColorBuffer[0]; }
		inline const GLfloat* getDirectionalLightDirectionBuffer() { return &directionalLightDirectionBuffer[0]; }
		inline const GLfloat* getDirectionalLightColorBuffer() { return &directionalLightColorBuffer[0]; }

	};

}
