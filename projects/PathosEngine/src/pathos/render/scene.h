#pragma once

#include "pathos/light/light.h"
#include "glm/glm.hpp"
#include <vector>

namespace pathos {

	// Forward declaration
	class Mesh;
	class DirectionalLight;
	class PointLight;
	class SkyRendering;

	// Represents a 3D scene.
	class Scene {

	public:
		Scene();
		Scene(const Scene&)            = delete;
		Scene& operator=(const Scene&) = delete;

		inline void add(Mesh* mesh) { meshes.push_back(mesh); }
		void add(std::initializer_list<Mesh*> meshes);
		
		inline void add(DirectionalLight* light) { directionalLights.push_back(light); }
		inline void add(PointLight* light)       { pointLights.push_back(light);       }

		inline uint32 numDirectionalLights() const { return (uint32)directionalLights.size(); }
		inline uint32 numPointLights()       const { return (uint32)pointLights.size();       }

		void calculateLightBuffer(); // in world space
		void calculateLightBufferInViewSpace(const glm::mat4& viewMatrix);

		inline const GLfloat* getPointLightBuffer() const { return (GLfloat*)pointLightBuffer.data(); }
		inline uint32 getPointLightBufferSize() const { return (uint32)(sizeof(PointLightProxy) * pointLightBuffer.size()); }

		inline const GLfloat* getDirectionalLightBuffer() const { return (GLfloat*)directionalLightBuffer.data(); }
		inline uint32 getDirectionalLightBufferSize() const { return (uint32)(sizeof(DirectionalLightProxy) * directionalLightBuffer.size()); }

	public:
		SkyRendering* sky = nullptr;
		Mesh* godRaySource = nullptr;
		std::vector<Mesh*> meshes;

		std::vector<DirectionalLight*> directionalLights;
		std::vector<PointLight*> pointLights;
		GLuint irradianceMap = 0;

	protected:
		std::vector<PointLightProxy> pointLightBuffer;
		std::vector<DirectionalLightProxy> directionalLightBuffer;

	};

}
