#include "pathos/mesh/geometry.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include "glm/glm.hpp"
#include <vector>

namespace pathos {

	static constexpr GLuint positionLocation  = 0;
	static constexpr GLuint uvLocation        = 1;
	static constexpr GLuint normalLocation    = 2;
	static constexpr GLuint tangentLocation   = 3;
	static constexpr GLuint bitangentLocation = 4;

	/////////////////////////////////////////////////////////////////////////////////////
	// MeshGeometry
	MeshGeometry::MeshGeometry()
		: drawArraysMode(false)
	{
	}

	MeshGeometry::~MeshGeometry() {
		dispose();
	}

	uint32 MeshGeometry::getIndexCount() { return indexCount; }

	void MeshGeometry::drawPrimitive(RenderCommandList& cmdList) {
		if (drawArraysMode){
			cmdList.drawArrays(GL_TRIANGLES, 0, positionCount);
		}else{
			cmdList.drawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
		}
	}

#define DEFINE_ACTIVATE_VAO(combination)                                    \
	void MeshGeometry::activate_##combination(RenderCommandList& cmdList) { \
		if(vao_##combination == 0) createVAO_##combination();				\
		cmdList.bindVertexArray(vao_##combination);							\
	}																		
	DEFINE_ACTIVATE_VAO(position)
	DEFINE_ACTIVATE_VAO(position_uv)
	DEFINE_ACTIVATE_VAO(position_normal)
	DEFINE_ACTIVATE_VAO(position_uv_normal)
	DEFINE_ACTIVATE_VAO(position_uv_normal_tangent_bitangent)
#undef DEFINE_ACTIVATE_VAO
	
	void MeshGeometry::deactivate(RenderCommandList& cmdList) {
		cmdList.bindVertexArray(0);
	}
	
	void MeshGeometry::updatePositionData(GLfloat* data, uint32 length) {
		positionData.assign(data, data + length);
		positionCount = length;
		if (!positionBuffer) {
			glGenBuffers(1, &positionBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), positionData.data(), GL_STATIC_DRAW);
	}
	void MeshGeometry::updateUVData(GLfloat* data, uint32 length) {
		uvData.assign(data, data + length);
		uvCount = length;
		if (!uvBuffer) {
			glGenBuffers(1, &uvBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), uvData.data(), GL_STATIC_DRAW);
	}
	void MeshGeometry::updateNormalData(GLfloat* data, uint32 length)
	{
		normalData.assign(data, data + length);
		normalCount = length;
		if (!normalBuffer) {
			glGenBuffers(1, &normalBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), normalData.data(), GL_STATIC_DRAW);
	}

	void MeshGeometry::updateTangentData(GLfloat* data, uint32 length) {
		tangentData.assign(data, data + length);
		if (!tangentBuffer){
			glGenBuffers(1, &tangentBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), tangentData.data(), GL_STATIC_DRAW);
	}
	void MeshGeometry::updateBitangentData(GLfloat* data, uint32 length) {
		bitangentData.assign(data, data + length);
		if (!bitangentBuffer){
			glGenBuffers(1, &bitangentBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), bitangentData.data(), GL_STATIC_DRAW);
	}
	void MeshGeometry::updateIndexData(GLuint* data, uint32 length) {
		indexData.assign(data, data + length);
		indexCount = length;
		if (!indexBuffer) {
			glGenBuffers(1, &indexBuffer);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, length * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW);
	}

	void MeshGeometry::activateIndexBuffer(RenderCommandList& cmdList) {
		cmdList.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	}
	void MeshGeometry::deactivateIndexBuffer(RenderCommandList& cmdList) {
		cmdList.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// requirements: positionData and indexData should be available.
	void MeshGeometry::calculateNormals() {
		if (drawArraysMode) calculateNormals_array();
		else calculateNormals_indexed();
	}
	void MeshGeometry::calculateNormals_array() {
		if (positionCount == 0 || positionData.size() == 0) {
			LOG(LogError, "%s: Position data should be available to calculate normals", __FUNCTION__);
			return;
		}

		std::vector<GLfloat> normals(positionCount, 0.0f);

		const GLfloat* P = positionData.data();
		for (uint32 i = 0u; i < positionCount; i += 9){
			uint32 p0 = i, p1 = i + 3, p2 = i + 6;
			glm::vec3 a = glm::vec3(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			glm::vec3 b = glm::vec3(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);
			glm::vec3 norm = glm::normalize(glm::cross(a, b));
			normals[i + 0] = normals[i + 3] = normals[i + 6] = norm.x;
			normals[i + 1] = normals[i + 4] = normals[i + 7] = norm.y;
			normals[i + 2] = normals[i + 5] = normals[i + 8] = norm.z;
		}

		updateNormalData(normals.data(), positionCount);
	}
	void MeshGeometry::calculateNormals_indexed() {
		if (positionCount == 0 || positionData.size() == 0) {
			LOG(LogError, "%s: Position data should be available to calculate normals", __FUNCTION__);
			return;
		}

		uint32 numPos = positionCount / 3;
		std::vector<glm::vec3> accum(numPos, glm::vec3(0.0f));
		std::vector<uint32> counts(numPos, 0);

		const std::vector<GLfloat>& P = positionData;

		for (uint32 i = 0u; i < indexCount; i += 3){
			uint32 i0 = indexData[i], i1 = indexData[i + 1], i2 = indexData[i + 2];
			uint32 p0 = i0 * 3, p1 = i1 * 3, p2 = i2 * 3;
			glm::vec3 a(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			glm::vec3 b(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);
			if (a == b) {
				continue;
			}
			glm::vec3 norm = glm::normalize(glm::cross(a, b));

			accum[i0] *= counts[i0];
			accum[i1] *= counts[i1];
			accum[i2] *= counts[i2];
			accum[i0] += norm;
			accum[i1] += norm;
			accum[i2] += norm;
			counts[i0] ++;
			counts[i1] ++;
			counts[i2] ++;
			accum[i0] /= counts[i0];
			accum[i1] /= counts[i1];
			accum[i2] /= counts[i2];
		}

		std::vector<GLfloat> normals(positionCount, 0.0f);
		for (uint32 i = 0; i < numPos; i++){
			accum[i] = glm::normalize(accum[i]);
			normals[i * 3 + 0] = accum[i].x;
			normals[i * 3 + 1] = accum[i].y;
			normals[i * 3 + 2] = accum[i].z;
		}
		updateNormalData(normals.data(), positionCount);
	}

	void MeshGeometry::calculateTangentBasis() {
		int32 numPos = positionCount / 3;
		std::vector<glm::vec3> accum(numPos, glm::vec3(0.0f)); // tangents
		std::vector<glm::vec3> accum2(numPos, glm::vec3(0.0f)); // bitangents

		const std::vector<GLfloat>& P = positionData;
		const std::vector<GLfloat>& UV = uvData;

		for (uint32 i = 0u; i < indexCount; i += 3) {
			GLuint i0 = indexData[i], i1 = indexData[i + 1], i2 = indexData[i + 2];
			GLuint p0 = i0 * 3, p1 = i1 * 3, p2 = i2 * 3;
			GLuint uv0 = i0 * 2, uv1 = i1 * 2, uv2 = i2 * 2;

			// delta position
			glm::vec3 dp1 = glm::vec3(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			glm::vec3 dp2 = glm::vec3(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);

			// delta uv
			glm::vec2 duv1 = glm::vec2(UV[uv1] - UV[uv0], UV[uv1 + 1] - UV[uv0 + 1]);
			glm::vec2 duv2 = glm::vec2(UV[uv2] - UV[uv0], UV[uv2 + 1] - UV[uv0 + 1]);
			float r = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);

			glm::vec3 tangent = (dp1 * duv2.y - dp2 * duv1.y) * r;
			glm::vec3 bitangent = (dp2 * duv1.x - dp1 * duv2.x) * r;

			accum[i0] += tangent;
			accum[i1] += tangent;
			accum[i2] += tangent;

			accum2[i0] += bitangent;
			accum2[i1] += bitangent;
			accum2[i2] += bitangent;
		}

		std::vector<GLfloat> tangents(positionCount, 0.0f);
		std::vector<GLfloat> bitangents(positionCount, 0.0f);

		for (int32 i = 0; i < numPos; i++) {
			glm::vec3 v = glm::normalize(accum[i]);
			tangents[i * 3 + 0] = v.x;
			tangents[i * 3 + 1] = v.y;
			tangents[i * 3 + 2] = v.z;

			v = glm::normalize(accum2[i]);
			bitangents[i * 3 + 0] = v.x;
			bitangents[i * 3 + 1] = v.y;
			bitangents[i * 3 + 2] = v.z;
		}

		updateTangentData(tangents.data(), positionCount);
		updateBitangentData(bitangents.data(), positionCount);
	}

	void MeshGeometry::dispose() {
		std::vector<GLuint> VBO;
		if (positionBuffer != 0)  VBO.push_back(positionBuffer);
		if (uvBuffer != 0)        VBO.push_back(uvBuffer);
		if (normalBuffer != 0)    VBO.push_back(normalBuffer);
		if (tangentBuffer != 0)   VBO.push_back(tangentBuffer);
		if (bitangentBuffer != 0) VBO.push_back(bitangentBuffer);
		if (indexBuffer != 0)     VBO.push_back(indexBuffer);
		glDeleteBuffers(static_cast<GLsizei>(VBO.size()), VBO.data());

		std::vector<GLuint> VAO;
		if (vao_position != 0)                             VAO.push_back(vao_position);
		if (vao_position_uv != 0)                          VAO.push_back(vao_position_uv);
		if (vao_position_normal != 0)                      VAO.push_back(vao_position_normal);
		if (vao_position_uv_normal != 0)                   VAO.push_back(vao_position_uv_normal);
		if (vao_position_uv_normal_tangent_bitangent != 0) VAO.push_back(vao_position_uv_normal_tangent_bitangent);
		glDeleteVertexArrays(static_cast<GLsizei>(VAO.size()), VAO.data());

#if 0
		// #todo: fix
		if (positionData) { delete[] positionData; positionData = nullptr; }
		if (indexData) { delete[] indexData; indexData = nullptr; }
		if (normalData) { delete[] normalData; normalData = nullptr; }
		if (uvData) { delete[] uvData; uvData = nullptr; }
		if (tangentData) { delete[] tangentData; tangentData = nullptr; }
		if (bitangentData) { delete[] bitangentData; bitangentData = nullptr; }
#endif
	}

	struct VAOElement {
		GLuint buffer;
		GLuint index;
		GLint size;
		GLenum type;
		GLboolean normalized;
	};

	static void createVAO(GLuint* vao, const std::vector<VAOElement>& descs, const char* debugLabel) {
		CHECK(*vao == 0);
		glGenVertexArrays(1, vao);
		CHECK(*vao != 0);
		glBindVertexArray(*vao);
		for (const VAOElement& desc : descs) {
			glBindBuffer(GL_ARRAY_BUFFER, desc.buffer);
			glVertexAttribPointer(desc.index, desc.size, desc.type, desc.normalized, 0, (void*)0);
			glEnableVertexAttribArray(desc.index);
			glObjectLabel(GL_VERTEX_ARRAY, *vao, -1, debugLabel);
		}
		glBindVertexArray(0);
	}

	void MeshGeometry::createVAO_position() {
		createVAO(&vao_position, {
			{ positionBuffer, positionLocation, 3, GL_FLOAT, GL_FALSE }
		}, "VAO_position");
	}

	void MeshGeometry::createVAO_position_uv() {
		createVAO(&vao_position_uv, {
			{ positionBuffer, positionLocation, 3, GL_FLOAT, GL_FALSE },
			{ uvBuffer,       uvLocation,       2, GL_FLOAT, GL_FALSE }
		}, "VAO_position_uv");
	}

	void MeshGeometry::createVAO_position_normal() {
		createVAO(&vao_position_normal, {
			{ positionBuffer, positionLocation, 3, GL_FLOAT, GL_FALSE },
			{ normalBuffer,   normalLocation,   3, GL_FLOAT, GL_FALSE }
		}, "VAO_position_normal");
	}

	void MeshGeometry::createVAO_position_uv_normal() {
		createVAO(&vao_position_uv_normal, {
			{ positionBuffer, positionLocation, 3, GL_FLOAT, GL_FALSE },
			{ uvBuffer,       uvLocation,       2, GL_FLOAT, GL_FALSE },
			{ normalBuffer,   normalLocation,   3, GL_FLOAT, GL_FALSE }
		}, "VAO_position_uv_normal");
	}

	void MeshGeometry::createVAO_position_uv_normal_tangent_bitangent() {
		createVAO(&vao_position_uv_normal_tangent_bitangent, {
			{ positionBuffer,  positionLocation,  3, GL_FLOAT, GL_FALSE },
			{ uvBuffer,        uvLocation,        2, GL_FLOAT, GL_FALSE },
			{ normalBuffer,    normalLocation,    3, GL_FLOAT, GL_FALSE },
			{ tangentBuffer,   tangentLocation,   3, GL_FLOAT, GL_FALSE },
			{ bitangentBuffer, bitangentLocation, 3, GL_FLOAT, GL_FALSE }
		}, "VAO_position_uv_normal_tangent_bitangent");
	}

}
