#pragma once

#include "geometry.h"

#include "badger/types/vector_types.h"
#include <vector>

namespace pathos {

	// Written for debugging. Does not support UVs and normals.
	class ProceduralGeometry : public MeshGeometry {
		
	public:
		ProceduralGeometry() {
			initializeVertexLayout(EVertexAttributes::All);
		}
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

		void addTriangle(const vector3& a, const vector3& b, const vector3& c) {
			positions.push_back(a);
			positions.push_back(b);
			positions.push_back(c);

			uvs.push_back(vector2(0.0f));
			uvs.push_back(vector2(0.0f));
			uvs.push_back(vector2(0.0f));

			uint32 i = (uint32)indices.size();
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}

		void addQuad(const vector3& a, const vector3& b, const vector3& c, const vector3& d) {
			addTriangle(a, b, c);
			addTriangle(a, c, d);
		}

	private:
		// #todo-geometry: Duplicate of superclass members?
		std::vector<vector3> positions;
		std::vector<vector2> uvs;
		std::vector<uint32> indices;

	};

}
