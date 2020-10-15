#pragma once

#include "geometry.h"
#include <vector>

namespace pathos {

	// Written for debugging. Does not support UVs and normals.
	class ProceduralGeometry : public MeshGeometry {
		
	public:
		ProceduralGeometry() {}
		~ProceduralGeometry() {}

		void clear() {
			positions.clear();
			uvs.clear();
			indices.clear();
		}

		void upload() {
			updatePositionData((GLfloat*)positions.data(), (uint32)(positions.size() * 3));
			updateUVData((GLfloat*)uvs.data(), (uint32)(uvs.size() * 2));
			updateIndexData((GLuint*)indices.data(), (uint32)indices.size());
		}

		void addTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
			positions.push_back(a);
			positions.push_back(b);
			positions.push_back(c);

			uvs.push_back(glm::vec2(0.0f));
			uvs.push_back(glm::vec2(0.0f));
			uvs.push_back(glm::vec2(0.0f));

			uint32 i = (uint32)indices.size();
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}

		void addQuad(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
			addTriangle(a, b, c);
			addTriangle(a, c, d);
		}

	private:
		// #todo-geometry: Duplicate of superclass members?
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uvs;
		std::vector<uint32> indices;

	};

}
