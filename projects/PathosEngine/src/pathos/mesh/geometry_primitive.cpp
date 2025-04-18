#include "geometry_primitive.h"

// NOTE: All triangles are counter-clockwise w.r.t OpenGL coordinate system.

namespace pathos {

	static MeshGeometry::EVertexAttributes toVertexAttributes(EPrimitiveInitOptions initOptions) {
		MeshGeometry::EVertexAttributes attributes = MeshGeometry::EVertexAttributes::None;
		if (ENUM_HAS_FLAG(initOptions, EPrimitiveInitOptions::CalculatePosition)) {
			attributes |= MeshGeometry::EVertexAttributes::Position;
		}
		if (ENUM_HAS_FLAG(initOptions, EPrimitiveInitOptions::CalculateUV)) {
			attributes |= MeshGeometry::EVertexAttributes::Uv;
		}
		if (ENUM_HAS_FLAG(initOptions, EPrimitiveInitOptions::CalculateNormal)) {
			attributes |= MeshGeometry::EVertexAttributes::Normal;
		}
		if (ENUM_HAS_FLAG(initOptions, EPrimitiveInitOptions::CalculateTangentBasis)) {
			attributes |= MeshGeometry::EVertexAttributes::Tangent;
			attributes |= MeshGeometry::EVertexAttributes::Bitangent;
		}
		return attributes;
	}

}

namespace pathos {

	/////////////////////////////////////////////////////////////////////////////////////
	// PlaneGeometry
	void PlaneGeometry::generate(const Input& input, Output& output) {
		float width = input.width;
		float height = input.height;
		uint32 gridX = input.gridX;
		uint32 gridY = input.gridY;
		PlaneGeometry::Direction direction = input.direction;
		EPrimitiveInitOptions options = input.options;
		std::vector<float>& outPositions = output.positions;
		std::vector<float>& outUVs = output.texcoords;
		std::vector<float>& outNormals = output.normals;
		std::vector<uint32>& outIndices = output.indices;

		const uint32 numVertices = (gridX + 1) * (gridY + 1);
		const float segW = width / gridX, segH = height / gridY;
		const float x0 = -0.5f * width, y0 = -0.5f * height;

		const bool bCalculateUV = ENUM_HAS_FLAG(options, EPrimitiveInitOptions::CalculateUV);
		const bool bCalculateNormal = ENUM_HAS_FLAG(options, EPrimitiveInitOptions::CalculateNormal);

		CHECKF(ENUM_HAS_FLAG(options, EPrimitiveInitOptions::CalculatePosition), "Position must be calculated");
		outPositions.resize(numVertices * 3);
		if (bCalculateUV) outUVs.resize(numVertices * 2);
		if (bCalculateNormal) outNormals.resize(numVertices * 3);
		outIndices.resize(gridX * gridY * 6);

		// Calculate attributes assuming Direction::Z
		size_t k = 0;
		for (auto i = 0u; i <= gridY; i++) {
			for (auto j = 0u; j <= gridX; j++) {
				outPositions[size_t(k * 3)] = x0 + segW * j;
				outPositions[size_t(k * 3 + 1)] = y0 + segH * i;
				outPositions[size_t(k * 3 + 2)] = 0.0f;
				if (bCalculateUV) {
					// #todo-geometry: Normalize or repeat UV?
					outUVs[size_t(k * 2)] = (float)j / gridX;
					outUVs[size_t(k * 2 + 1)] = (float)i / gridY;
					//outUVs[size_t(k * 2)] = (float)j;
					//outUVs[size_t(k * 2 + 1)] = (float)i;
				}
				if (bCalculateNormal) {
					outNormals[size_t(k * 3)] = 0.0f;
					outNormals[size_t(k * 3 + 1)] = 0.0f;
					outNormals[size_t(k * 3 + 2)] = 1.0f;
				}
				k++;
			}
		}

		// If not Direction::Z, then rotate the attributes.
		if (direction == PlaneGeometry::Direction::X) {
			for (auto i = 0u; i < outPositions.size(); i += 3) {
				outPositions[size_t(i + 2)] = -outPositions[i];
				outPositions[i] = 0.0f;
				if (bCalculateNormal) {
					outNormals[i] = 1.0f;
					outNormals[size_t(i + 1)] = outNormals[size_t(i + 2)] = 0.0f;
				}
			}
		} else if (direction == PlaneGeometry::Direction::Y) {
			for (auto i = 0u; i < outPositions.size(); i += 3) {
				outPositions[size_t(i + 2)] = -outPositions[size_t(i + 1)];
				outPositions[size_t(i + 1)] = 0.0f;
				if (bCalculateNormal) {
					outNormals[size_t(i + 1)] = 1.0f;
					outNormals[i] = outNormals[size_t(i + 2)] = 0.0f;
				}
			}
		}

		k = 0;
		for (auto i = 0u; i < gridY; i++) {
			auto baseY = i * (gridX + 1);
			for (auto j = 0u; j < gridX; j++) {
				outIndices[k] = baseY + j;
				outIndices[k + 1] = baseY + j + 1;
				outIndices[k + 2] = baseY + j + (gridX + 1);
				outIndices[k + 3] = baseY + j + 1;
				outIndices[k + 4] = baseY + j + (gridX + 1) + 1;
				outIndices[k + 5] = baseY + j + (gridX + 1);
				k += 6;
			}
		}
	}

	PlaneGeometry::PlaneGeometry(const Input& input){
		Output output;
		PlaneGeometry::generate(input, output);

		const bool bCalculateUV = ENUM_HAS_FLAG(input.options, EPrimitiveInitOptions::CalculateUV);
		const bool bCalculateNormal = ENUM_HAS_FLAG(input.options, EPrimitiveInitOptions::CalculateNormal);

		initializeVertexLayout(toVertexAttributes(input.options));

		updatePositionData(output.positions.data(), (uint32)output.positions.size());
		if (bCalculateUV) updateUVData(output.texcoords.data(), (uint32)output.texcoords.size());
		if (bCalculateNormal) updateNormalData(output.normals.data(), (uint32)output.normals.size());
		updateIndexData(output.indices.data(), (uint32)output.indices.size());

		if (ENUM_HAS_FLAG(input.options, EPrimitiveInitOptions::CalculateTangentBasis)) {
			calculateTangentBasis();
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// CubeGeometry
	CubeGeometry::CubeGeometry(
		const vector3& inHalfSize,
		bool smoothing,
		EPrimitiveInitOptions options)
		: halfSize(inHalfSize)
	{
		initializeVertexLayout(toVertexAttributes(options));
		buildGeometry(smoothing);
		if (ENUM_HAS_FLAG(options, EPrimitiveInitOptions::CalculateTangentBasis)) {
			calculateTangentBasis();
		}
	}

	void CubeGeometry::buildGeometry(bool smoothing) {
		if (smoothing) {
			buildGeometry_smoothing();
		} else {
			buildGeometry_noSmoothing();
		}
	}

	void CubeGeometry::buildGeometry_smoothing() {
		float hx = halfSize.x, hy = halfSize.y, hz = halfSize.z;
		GLfloat positions[24] = {
			hx, hy, hz,
			hx, -hy, hz,
			-hx, -hy, hz,
			-hx, hy, hz,
			hx, hy, -hz,
			hx, -hy, -hz,
			-hx, -hy, -hz,
			-hx, hy, -hz
		};
		GLfloat uvs[16] = { 1,1, 1,0, 0,0, 0,1, 1,1, 1,0, 0,0, 0,1 };
		GLfloat normals[24] = {
			1, 1, 1,
			1, -1, 1,
			-1, -1, 1,
			-1, 1, 1,
			1, 1, -1,
			1, -1, -1,
			-1, -1, -1,
			-1, 1, -1 };
		GLuint indices[36] = {
			0,2,1, 0,3,2,
			4,1,5, 4,0,1,
			4,3,0, 4,7,3,
			3,6,2, 3,7,6,
			7,5,6, 7,4,5,
			1,6,5, 1,2,6 };

		updatePositionData(positions, 24);
		updateUVData(uvs, 16);
		updateNormalData(normals, 24);
		updateIndexData(indices, 36);
	}

	void CubeGeometry::buildGeometry_noSmoothing() {
		float hx = halfSize.x, hy = halfSize.y, hz = halfSize.z;
		
		vector3 corners[8] = {
			{hx, hy, hz},
			{hx, -hy, hz},
			{-hx, -hy, hz},
			{-hx, hy, hz},
			{hx, hy, -hz},
			{hx, -hy, -hz},
			{-hx, -hy, -hz},
			{-hx, hy, -hz}
		};
		GLuint sourceIndices[36] = {
			0,2,1, 0,3,2,
			4,1,5, 4,0,1,
			4,3,0, 4,7,3,
			3,6,2, 3,7,6,
			7,5,6, 7,4,5,
			1,6,5, 1,2,6 };

		std::vector<GLfloat> positions;
		std::vector<GLfloat> uvs;
		std::vector<GLfloat> normals;
		std::vector<GLuint> indices;

#define PUSH_VEC3(vs, v) { vs.push_back(v.x); vs.push_back(v.y); vs.push_back(v.z); }

		// #todo-geometry: Unnecessary runtime generation. Use literals.
		int32 k = 0;
		for (size_t i = 0; i < 36; i += 3)
		{
			GLuint j0 = sourceIndices[i + 0];
			GLuint j1 = sourceIndices[i + 1];
			GLuint j2 = sourceIndices[i + 2];
			vector3 P0 = corners[j0];
			vector3 P1 = corners[j1];
			vector3 P2 = corners[j2];
			vector3 N = glm::normalize(glm::cross(P1 - P0, P2 - P0));

			PUSH_VEC3(positions, P0);
			PUSH_VEC3(positions, P1);
			PUSH_VEC3(positions, P2);
			if (i % 6 == 0) {
				uvs.push_back(1.0f); uvs.push_back(1.0f);
				uvs.push_back(0.0f); uvs.push_back(0.0f);
				uvs.push_back(1.0f); uvs.push_back(0.0f);
			} else {
				uvs.push_back(1.0f); uvs.push_back(1.0f);
				uvs.push_back(0.0f); uvs.push_back(1.0f);
				uvs.push_back(0.0f); uvs.push_back(0.0f);
			}
			PUSH_VEC3(normals, N);
			PUSH_VEC3(normals, N);
			PUSH_VEC3(normals, N);
			indices.push_back(k);
			indices.push_back(k + 1);
			indices.push_back(k + 2);
			k += 3;
		}

#undef PUSH_VEC3

		updatePositionData(positions.data(), (uint32)positions.size());
		updateUVData(uvs.data(), (uint32)uvs.size());
		updateNormalData(normals.data(), (uint32)normals.size());
		updateIndexData(indices.data(), (uint32)indices.size());
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// SphereGeometry

	void SphereGeometry::generate(const Input& input, Output& output) {
		float radius = input.radius;
		uint32 division = input.subdivision;
		EPrimitiveInitOptions options = input.options;
		std::vector<float>& outPositions = output.positions;
		std::vector<float>& outUVs = output.texcoords;
		std::vector<float>& outNormals = output.normals;
		std::vector<uint32>& outIndices = output.indices;

		const uint32 numVertices = division * division;
		const uint32 numTriangles = division * (division - 1) * 2;
		const bool bCalculateUV = ENUM_HAS_FLAG(options, EPrimitiveInitOptions::CalculateUV);
		const bool bCalculateNormal = ENUM_HAS_FLAG(options, EPrimitiveInitOptions::CalculateNormal);

		outPositions.resize(numVertices * 3);
		if (bCalculateUV) outUVs.resize(numVertices * 2);
		if (bCalculateNormal) outNormals.resize(numVertices * 3);
		outIndices.resize(numTriangles * 3);

		float pi = atan(1.f) * 4.f;
		float pi_2 = pi * .5f, pi2 = pi + pi;
		float theta, phi;
		float x, y, z;
		int k = 0;

		for (size_t i = 0; i < division; i++) {
			phi = pi_2 - pi * (float)(i) / (division - 1);
			for (size_t j = 0; j < division; j++) {
				theta = pi2 * (float)j / (division - 1);
				x = cos(phi) * cos(theta);
				y = cos(phi) * sin(theta);
				z = sin(phi);

				outPositions[size_t(3 * k + 0)] = radius * x;
				outPositions[size_t(3 * k + 1)] = radius * y;
				outPositions[size_t(3 * k + 2)] = radius * z;

				if (bCalculateNormal) {
					outNormals[size_t(3 * k + 0)] = x;
					outNormals[size_t(3 * k + 1)] = y;
					outNormals[size_t(3 * k + 2)] = z;
				}
				if (bCalculateUV) {
					outUVs[size_t(2 * k + 0)] = theta / pi2;
					outUVs[size_t(2 * k + 1)] = (phi + pi_2) / pi;
				}

				if (i != division - 1) {
					if (j != division - 1) {
						outIndices[size_t(6 * k + 0)] = k;
						outIndices[size_t(6 * k + 1)] = k + division;
						outIndices[size_t(6 * k + 2)] = k + division + 1;
						outIndices[size_t(6 * k + 3)] = k;
						outIndices[size_t(6 * k + 4)] = k + division + 1;
						outIndices[size_t(6 * k + 5)] = k + 1;
					} else {
						outIndices[size_t(6 * k + 0)] = k;
						outIndices[size_t(6 * k + 1)] = k + division;
						outIndices[size_t(6 * k + 2)] = k + 1;
						outIndices[size_t(6 * k + 3)] = k;
						outIndices[size_t(6 * k + 4)] = k + 1;
						outIndices[size_t(6 * k + 5)] = k + 1 - division;
					}
				}
				k++;
			}
		}
	}

	SphereGeometry::SphereGeometry(const Input& input) {
		Output output;
		SphereGeometry::generate(input, output);

		const bool bCalculateUV = ENUM_HAS_FLAG(input.options, EPrimitiveInitOptions::CalculateUV);
		const bool bCalculateNormal = ENUM_HAS_FLAG(input.options, EPrimitiveInitOptions::CalculateNormal);

		initializeVertexLayout(toVertexAttributes(input.options));

		updatePositionData(output.positions.data(), (uint32)output.positions.size());
		if (bCalculateUV) updateUVData(output.texcoords.data(), (uint32)output.texcoords.size());
		if (bCalculateNormal) updateNormalData(output.normals.data(), (uint32)output.normals.size());
		updateIndexData(output.indices.data(), (uint32)output.indices.size());

		if (ENUM_HAS_FLAG(input.options, EPrimitiveInitOptions::CalculateTangentBasis)) {
			calculateTangentBasis();
		}
	}

}
