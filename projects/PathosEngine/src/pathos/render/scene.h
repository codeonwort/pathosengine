#pragma once

#include "pathos/light/light.h"
#include "glm/glm.hpp"
#include <vector>

namespace pathos {

	// Forward declaration
	class Mesh;
	class DirectionalLight;
	class PointLight;
	class Skybox;

	// Represents a 3D scene.
	class Scene {

	public:
		Scene();
		Scene(Scene&&)                 = delete;
		Scene(const Scene&)            = delete;
		Scene& operator=(const Scene&) = delete;

		Skybox* skybox = nullptr;
		Mesh* godRaySource = nullptr;
		std::vector<Mesh*> meshes;

		std::vector<DirectionalLight*> directionalLights;
		std::vector<PointLight*> pointLights;

		inline void add(Mesh* mesh) { meshes.push_back(mesh); }
		void add(std::initializer_list<Mesh*> meshes);
		
		inline void add(DirectionalLight* light) { directionalLights.push_back(light); }
		inline void add(PointLight* light)       { pointLights.push_back(light);       }

		inline uint32_t numDirectionalLights() const { return (uint32_t)directionalLights.size(); }
		inline uint32_t numPointLights()       const { return (uint32_t)pointLights.size();       }

		void calculateLightBuffer(); // in world space
		void calculateLightBufferInViewSpace(const glm::mat4& viewMatrix);

		inline const GLfloat* getPointLightPositionBuffer()        const { return (GLfloat*)pointLightPositionBuffer.data();        }
		inline const GLfloat* getPointLightColorBuffer()           const { return (GLfloat*)pointLightColorBuffer.data();           }
		inline const GLfloat* getDirectionalLightDirectionBuffer() const { return (GLfloat*)directionalLightDirectionBuffer.data(); }
		inline const GLfloat* getDirectionalLightColorBuffer()     const { return (GLfloat*)directionalLightColorBuffer.data();     }

		inline uint32_t getPointLightBufferSize()       const { return (uint32_t)(sizeof(glm::vec4) * pointLightPositionBuffer.size());        }
		inline uint32_t getDirectionalLightBufferSize() const { return (uint32_t)(sizeof(glm::vec4) * directionalLightDirectionBuffer.size()); }

	protected:
		// w components are not used.
		std::vector<glm::vec4> pointLightPositionBuffer;
		std::vector<glm::vec4> pointLightColorBuffer;
		std::vector<glm::vec4> directionalLightDirectionBuffer;
		std::vector<glm::vec4> directionalLightColorBuffer;

	};

}
