#include "ansel_envmap.h"
#include "pathos/render/shader.h"
#include "glm/gtc/constants.hpp"
#include <string>
#include <cassert>

namespace pathos {

	IcosahedronGeometry::IcosahedronGeometry(unsigned int subdivisionSteps) {
		buildGeometry();
		while (subdivisionSteps-- > 0) subdivide();
		//buildNormalAndUV();
		uploadToGPU();
	}

	void IcosahedronGeometry::buildGeometry() {
		const float X = 0.525731112119133606f;
		const float Z = 0.850650808352039932f;
		const float pi_inv = glm::one_over_pi<float>();

		numVertices = INITIAL_NUM_VERTICES;
		numTriangles = INITIAL_NUM_TRIANLGES;

		GLfloat positionDataSource[INITIAL_NUM_VERTICES][3] = {
			{ -X, 0.0, Z },{ X, 0.0, Z },{ -X, 0.0, -Z },{ X, 0.0, -Z },
			{ 0.0, Z, X },{ 0.0, Z, -X },{ 0.0, -Z, X },{ 0.0, -Z, -X },
			{ Z, X, 0.0 },{ -Z, X, 0.0 },{ Z, -X, 0.0 },{ -Z, -X, 0.0 }
		};
		// cw winding
		GLuint indexDataSource[INITIAL_NUM_TRIANLGES][3] = {
			{ 0,4,1 },{ 0,9,4 },{ 9,5,4 },{ 4,5,8 },{ 4,8,1 },
			{ 8,10,1 },{ 8,3,10 },{ 5,3,8 },{ 5,2,3 },{ 2,7,3 },
			{ 7,10,3 },{ 7,6,10 },{ 7,11,6 },{ 11,0,6 },{ 0,1,6 },
			{ 6,1,10 },{ 9,0,11 },{ 9,11,2 },{ 9,2,5 },{ 7,2,11 }
		};

		positionData = new GLfloat[numVertices * 3];
		indexData = new GLuint[numTriangles * 3];

		memcpy(positionData, positionDataSource, sizeof(GLfloat) * numVertices * 3);
		memcpy(indexData, indexDataSource, sizeof(GLuint) * numTriangles * 3);
	}

	void IcosahedronGeometry::subdivide() {
		unsigned int newNumVertices = numVertices + 3 * numTriangles; // three vertices are created per one triangle
		unsigned int newNumTriangles = numTriangles * 4; // each triangle is divided into four triangles

		GLfloat* newPositions = new GLfloat[newNumVertices * 3];
		GLuint* newIndices = new GLuint[newNumTriangles * 3];

		memcpy(newPositions, positionData, sizeof(GLfloat) * numVertices * 3);

		// build
		unsigned int index_end = numTriangles * 3;
		unsigned int p = numVertices; // position append location
		unsigned int q = 0; // index append location
		for (unsigned int i = 0; i < index_end; i += 3) {
			unsigned int i0 = indexData[i];
			unsigned int i1 = indexData[i + 1];
			unsigned int i2 = indexData[i + 2];

			unsigned int i01, i12, i20;
			i01 = p;
			i12 = p + 1;
			i20 = p + 2;

			glm::vec3 v0(positionData[i0 * 3], positionData[i0 * 3 + 1], positionData[i0 * 3 + 2]);
			glm::vec3 v1(positionData[i1 * 3], positionData[i1 * 3 + 1], positionData[i1 * 3 + 2]);
			glm::vec3 v2(positionData[i2 * 3], positionData[i2 * 3 + 1], positionData[i2 * 3 + 2]);

			glm::vec3 v01 = glm::normalize(v0 + v1);
			glm::vec3 v12 = glm::normalize(v1 + v2);
			glm::vec3 v20 = glm::normalize(v2 + v0);

			assert(glm::length<float>(v01) > 0.00001f);
			assert(glm::length<float>(v12) > 0.00001f);
			assert(glm::length<float>(v20) > 0.00001f);

			newPositions[i01 * 3] = v01.x;
			newPositions[i01 * 3 + 1] = v01.y;
			newPositions[i01 * 3 + 2] = v01.z;

			newPositions[i12 * 3] = v12.x;
			newPositions[i12 * 3 + 1] = v12.y;
			newPositions[i12 * 3 + 2] = v12.z;

			newPositions[i20 * 3] = v20.x;
			newPositions[i20 * 3 + 1] = v20.y;
			newPositions[i20 * 3 + 2] = v20.z;

			newIndices[q++] = i0;
			newIndices[q++] = i01;
			newIndices[q++] = i20;

			newIndices[q++] = i1;
			newIndices[q++] = i12;
			newIndices[q++] = i01;

			newIndices[q++] = i12;
			newIndices[q++] = i2;
			newIndices[q++] = i20;

			newIndices[q++] = i01;
			newIndices[q++] = i12;
			newIndices[q++] = i20;

			p += 3;
		}

		assert(p == newNumVertices);
		assert(q == newNumTriangles * 3);

		// replace
		delete[] positionData;
		delete[] indexData;

		positionData = newPositions;
		indexData = newIndices;

		numVertices = newNumVertices;
		numTriangles = newNumTriangles;
	}

	/*
	void IcosahedronGeometry::buildNormalAndUV() {
		normalData = new GLfloat[numVertices * 3];
		uvData = new GLfloat[numVertices * 2];

		// normal: simple copy
		memcpy(normalData, positionData, sizeof(GLfloat) * numVertices * 3);

		// uv
		const float pi_inv = glm::one_over_pi<float>();
		for (int i = 0; i < numVertices; ++i) {
			float x = positionData[i * 3];
			float y = positionData[i * 3 + 1];
			float z = positionData[i * 3 + 2];

			glm::vec3 r(x, y, z);
			float u, v;

			v = r.y;
			r.y = 0.0f;
			if (glm::length<float>(r) <= 0.00001f) {
				u = 0.0f;
			} else {
				u = glm::normalize(r).x * 0.5;
			}

			float s = (r.z >= 0.0f ? 1.0f : -1.0f) * 0.5f;
			u = 0.75 - s * (0.5 - u);
			v = 0.5 + 0.5 * v;

			if (u > 1.0f) u -= 1.0f;

			uvData[i * 2] = u;
			uvData[i * 2 + 1] = v;
		}
	}
	*/

	void IcosahedronGeometry::uploadToGPU() {
		updatePositionData(positionData, numVertices * 3);
		//updateUVData(uvData, numVertices * 2);
		//updateNormalData(normalData, numVertices * 3);
		updateIndexData(indexData, numTriangles * 3);
	}

	//////////////////////////////////////////////////////////////////////////////////////

	AnselEnvMapping::AnselEnvMapping(GLuint textureID) :texture(textureID) {
		//sphere = new SphereGeometry(1.0f, 50);
		sphere = new IcosahedronGeometry(2);

		createShaderProgram();
	}

	AnselEnvMapping::~AnselEnvMapping() {
		delete sphere;
		sphere = nullptr;

		glDeleteProgram(program);
	}

	void AnselEnvMapping::render(const glm::mat4& transform) {
		// activate ///////////////////////////////////////////////////////
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_DEPTH_TEST);
		//glCullFace(GL_FRONT);

		sphere->activate_position();
		sphere->activateIndexBuffer();

		glUseProgram(program);
		glUniformMatrix4fv(uniform_transform, 1, GL_FALSE, &transform[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		// draw call ///////////////////////////////////////////////////////
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		sphere->draw();
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// deactivate ///////////////////////////////////////////////////////
		sphere->deactivate();
		sphere->deactivateIndexBuffer();

		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
		//glCullFace(GL_BACK);
	}

	void AnselEnvMapping::createShaderProgram() {
		std::string vs = R"(
#version 430 core

layout (location = 0) in vec3 position;
//layout (location = 1) in vec2 uv;

uniform mat4 viewProj;

out VS_OUT {
	//vec2 uv;
	vec3 r;
} vs_out;

void main() {
	//vs_out.uv = uv;
	vs_out.r = position;
	gl_Position = (viewProj * vec4(position, 1)).xyww;
}

)";

		std::string fs = R"(
#version 430 core

layout (binding = 0) uniform sampler2D texSampler;

in VS_OUT {
	//vec2 uv;
	vec3 r;
} fs_in;

layout (location = 0) out vec4 out_color;

void main() {
	vec3 r = normalize(fs_in.r);
	vec2 tc;
	tc.y = r.y; r.y = 0.0;
	tc.x = normalize(r).x * 0.5;
	float s = sign(r.z) * 0.5;
	tc.s = 0.75 - s * (0.5 - tc.s);
	tc.t = 0.5 + 0.5 * tc.t;
	out_color = texture(texSampler, tc);
	//out_color = texture(texSampler, fs_in.uv);
}

)";

		program = pathos::createProgram(vs, fs);
		assert(program != 0);
		uniform_transform = glGetUniformLocation(program, "viewProj");
	}

}