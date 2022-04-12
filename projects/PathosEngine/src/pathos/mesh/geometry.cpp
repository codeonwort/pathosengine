#include "pathos/mesh/geometry.h"
#include "pathos/util/log.h"
#include "pathos/render/render_device.h"

#include "badger/assertion/assertion.h"
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

	uint32 MeshGeometry::getIndexCount() const { return (uint32)indexData.size(); }

	void MeshGeometry::drawPrimitive(RenderCommandList& cmdList) {
		if (drawArraysMode) {
			cmdList.drawArrays(GL_TRIANGLES, 0, (GLsizei)positionData.size());
		} else {
			cmdList.drawElements(GL_TRIANGLES, (GLsizei)indexData.size(), GL_UNSIGNED_INT, (void*)0);
		}
	}

#define DEFINE_ACTIVATE_VAO(combination)                                    \
	void MeshGeometry::activate_##combination(RenderCommandList& cmdList) { \
		if(vao_##combination == 0) createVAO_##combination(cmdList);		\
		cmdList.bindVertexArray(vao_##combination);							\
	}																		
	DEFINE_ACTIVATE_VAO(position)
	DEFINE_ACTIVATE_VAO(position_uv)
	DEFINE_ACTIVATE_VAO(position_normal)
	DEFINE_ACTIVATE_VAO(position_uv_normal)
	DEFINE_ACTIVATE_VAO(position_uv_normal_tangent_bitangent)
#undef DEFINE_ACTIVATE_VAO

	static void enqueueBufferUpload(GLuint bufferName, GLsizeiptr size, const void* data) {
		// #todo-renderthread
#if 0
		CHECK(isInMainThread());
		ENQUEUE_RENDER_COMMAND([bufferName, size, data](RenderCommandList& cmdList) {
				cmdList.namedBufferData(bufferName, size, data, GL_STATIC_DRAW);
			}
		);
#else
		if (isInRenderThread()) {
			RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
			cmdList.namedBufferData(bufferName, size, data, GL_STATIC_DRAW);
		} else {
			ENQUEUE_RENDER_COMMAND([bufferName, size, data](RenderCommandList& cmdList)
				{
					cmdList.namedBufferData(bufferName, size, data, GL_STATIC_DRAW);
				}
			);
		}
#endif
	}

	void MeshGeometry::deactivate(RenderCommandList& cmdList) {
		cmdList.bindVertexArray(0);
	}
	
	void MeshGeometry::updatePositionData(const GLfloat* data, uint32 length) {
		CHECKF(length % 3 == 0, "Invalid length");
		const vector3* data2 = reinterpret_cast<const vector3*>(data);

		positionData.assign(data2, data2 + length / 3);
		if (!positionBuffer) {
			gRenderDevice->createBuffers(1, &positionBuffer);
		}
		enqueueBufferUpload(positionBuffer, length * sizeof(GLfloat), positionData.data());
	}

	void MeshGeometry::updateUVData(const GLfloat* data, uint32 length) {
		CHECKF(length % 2 == 0, "Invalid length");
		const vector2* data2 = reinterpret_cast<const vector2*>(data);

		uvData.assign(data2, data2 + length / 2);
		if (!uvBuffer) {
			gRenderDevice->createBuffers(1, &uvBuffer);
		}
		enqueueBufferUpload(uvBuffer, length * sizeof(GLfloat), uvData.data());
	}

	void MeshGeometry::updateNormalData(const GLfloat* data, uint32 length) {
		CHECKF(length % 3 == 0, "Invalid length");
		const vector3* data2 = reinterpret_cast<const vector3*>(data);

		normalData.assign(data2, data2 + length / 3);
		if (!normalBuffer) {
			gRenderDevice->createBuffers(1, &normalBuffer);
		}
		enqueueBufferUpload(normalBuffer, length * sizeof(GLfloat), normalData.data());
	}
	void MeshGeometry::updateNormalData(const std::vector<vector3>& inNormals)
	{
		const GLfloat* buffer = reinterpret_cast<const GLfloat*>(inNormals.data());
		uint32 length = (uint32)(inNormals.size() * 3);
		updateNormalData(buffer, length);
	}

	void MeshGeometry::updateTangentData(const GLfloat* data, uint32 length) {
		CHECKF(length % 3 == 0, "Invalid length");
		const vector3* data2 = reinterpret_cast<const vector3*>(data);

		tangentData.assign(data2, data2 + length / 3);
		if (!tangentBuffer) {
			gRenderDevice->createBuffers(1, &tangentBuffer);
		}
		enqueueBufferUpload(tangentBuffer, length * sizeof(GLfloat), tangentData.data());
	}
	void MeshGeometry::updateBitangentData(const GLfloat* data, uint32 length) {
		CHECKF(length % 3 == 0, "Invalid length");
		const vector3* data2 = reinterpret_cast<const vector3*>(data);

		bitangentData.assign(data2, data2 + length / 3);
		if (!bitangentBuffer) {
			gRenderDevice->createBuffers(1, &bitangentBuffer);
		}
		enqueueBufferUpload(bitangentBuffer, length * sizeof(GLfloat), bitangentData.data());
	}
	void MeshGeometry::updateIndexData(const GLuint* data, uint32 length) {
		indexData.assign(data, data + length);
		if (!indexBuffer) {
			gRenderDevice->createBuffers(1, &indexBuffer);
		}
		enqueueBufferUpload(indexBuffer, length * sizeof(GLuint), indexData.data());
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
		if (positionData.size() == 0) {
			LOG(LogError, "%s: Position data should be available to calculate normals", __FUNCTION__);
			return;
		}

		const uint32 numVertices = (uint32)positionData.size();
		std::vector<vector3> normals(numVertices, vector3(0.0f));

		for (uint32 i = 0u; i < numVertices; i += 3){
			vector3 a = positionData[i + 1] - positionData[i];
			vector3 b = positionData[i + 2] - positionData[i];
			vector3 norm = glm::normalize(glm::cross(a, b));
			normals[i] = normals[i + 1] = normals[i + 2] = norm;
		}

		updateNormalData(normals);
	}
	void MeshGeometry::calculateNormals_indexed() {
		if (positionData.size() == 0) {
			LOG(LogError, "%s: Position data should be available to calculate normals", __FUNCTION__);
			return;
		}

		const uint32 numPos = (uint32)(positionData.size());
		const uint32 indexCount = (uint32)indexData.size();
		std::vector<vector3> normals(numPos, vector3(0.0f));

		for (uint32 i = 0u; i < indexCount; i += 3) {
			uint32 i0 = indexData[i], i1 = indexData[i + 1], i2 = indexData[i + 2];

			vector3 a = positionData[i1] - positionData[i0];
			vector3 b = positionData[i2] - positionData[i0];
			if (a == b) {
				continue;
			}

			vector3 norm = glm::normalize(glm::cross(a, b));
			normals[i0] += norm;
			normals[i1] += norm;
			normals[i2] += norm;
		}

		for (uint32 i = 0; i < numPos; ++i) {
			normals[i] = glm::normalize(normals[i]);
		}
		updateNormalData(normals);
	}

	void MeshGeometry::calculateTangentBasis() {
		const int32 numPos = (int32)positionData.size();
		const uint32 indexCount = (uint32)indexData.size();

		std::vector<vector3> accum(numPos, vector3(0.0f)); // tangents
		std::vector<vector3> accum2(numPos, vector3(0.0f)); // bitangents

		for (uint32 i = 0u; i < indexCount; i += 3) {
			GLuint i0 = indexData[i], i1 = indexData[i + 1], i2 = indexData[i + 2];

			// delta position
			vector3 dp1 = positionData[i1] - positionData[i0];
			vector3 dp2 = positionData[i2] - positionData[i0];

			// delta uv
			vector2 duv1 = uvData[i1] - uvData[i0];
			vector2 duv2 = uvData[i2] - uvData[i0];
			float r = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);

			vector3 tangent   = glm::normalize( (dp1 * duv2.y - dp2 * duv1.y) * r );
			vector3 bitangent = glm::normalize( (dp2 * duv1.x - dp1 * duv2.x) * r );

			accum[i0] += tangent;
			accum[i1] += tangent;
			accum[i2] += tangent;

			accum2[i0] += bitangent;
			accum2[i1] += bitangent;
			accum2[i2] += bitangent;
		}

		std::vector<GLfloat> tangents(numPos * 3, 0.0f);
		std::vector<GLfloat> bitangents(numPos * 3, 0.0f);

		for (int32 i = 0; i < numPos; ++i) {
			vector3 v = glm::normalize(accum[i]);
			tangents[i * 3 + 0] = v.x;
			tangents[i * 3 + 1] = v.y;
			tangents[i * 3 + 2] = v.z;

			v = glm::normalize(accum2[i]);
			bitangents[i * 3 + 0] = v.x;
			bitangents[i * 3 + 1] = v.y;
			bitangents[i * 3 + 2] = v.z;
		}

		updateTangentData(tangents.data(), numPos * 3);
		updateBitangentData(bitangents.data(), numPos * 3);
	}

	void MeshGeometry::dispose() {
		std::vector<GLuint> VBO;
		if (positionBuffer != 0)  VBO.push_back(positionBuffer);
		if (uvBuffer != 0)        VBO.push_back(uvBuffer);
		if (normalBuffer != 0)    VBO.push_back(normalBuffer);
		if (tangentBuffer != 0)   VBO.push_back(tangentBuffer);
		if (bitangentBuffer != 0) VBO.push_back(bitangentBuffer);
		if (indexBuffer != 0)     VBO.push_back(indexBuffer);
		gRenderDevice->deleteBuffers(static_cast<GLsizei>(VBO.size()), VBO.data());

		std::vector<GLuint> VAO;
		if (vao_position != 0)                             VAO.push_back(vao_position);
		if (vao_position_uv != 0)                          VAO.push_back(vao_position_uv);
		if (vao_position_normal != 0)                      VAO.push_back(vao_position_normal);
		if (vao_position_uv_normal != 0)                   VAO.push_back(vao_position_uv_normal);
		if (vao_position_uv_normal_tangent_bitangent != 0) VAO.push_back(vao_position_uv_normal_tangent_bitangent);
		gRenderDevice->deleteVertexArrays(static_cast<GLsizei>(VAO.size()), VAO.data());
	}

	struct VAOElement {
		GLuint buffer;
		GLuint index;
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLsizei stride;
	};

	static void createVAO(
		RenderCommandList& cmdList,
		GLuint indexBuffer,
		GLuint* vao,
		const std::vector<VAOElement>& descs,
		const char* debugLabel)
	{
		CHECK(*vao == 0 && indexBuffer != 0);
		gRenderDevice->createVertexArrays(1, vao);
		gRenderDevice->objectLabel(GL_VERTEX_ARRAY, *vao, -1, debugLabel);

#if 1
		// attribindex and bindingindex are so confusing...
		for (const VAOElement& desc : descs) {
			CHECKF(desc.buffer != 0, "Null buffer is not allowed");

			cmdList.vertexArrayVertexBuffer(
				*vao,            // VAO
				desc.index,      // binding index
				desc.buffer,     // buffer name
				0,               // offset
				desc.stride      // stride
			);
			cmdList.vertexArrayElementBuffer(*vao, indexBuffer);

			cmdList.enableVertexArrayAttrib(
				*vao,            // VAO
				desc.index       // index of the generic vertex attribute
			);

			cmdList.vertexArrayAttribFormat(
				*vao,            // VAO
				desc.index,      // attribute index
				desc.size,       // the number of components
				desc.type,       // data type of components
				desc.normalized, // is normalized
				0                // relative offset (distance between elements within the buffer)
			);
			
			cmdList.vertexArrayAttribBinding(*vao, desc.index, desc.index); // vao, attribindex, bindingindex
		}
#else
		glBindVertexArray(*vao);
		for (const VAOElement& desc : descs) {
			CHECKF(desc.buffer != 0, "Null buffer is not allowed");
			glBindBuffer(GL_ARRAY_BUFFER, desc.buffer);
			glVertexAttribPointer(desc.index, desc.size, desc.type, desc.normalized, 0, (void*)0);
			glEnableVertexArrayAttrib(*vao, desc.index);
		}
		glBindVertexArray(0);
#endif
	}

	void MeshGeometry::createVAO_position(RenderCommandList& cmdList) {
		createVAO(cmdList, indexBuffer, &vao_position, {
			{ positionBuffer, positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) }
		}, "VAO_position");
	}

	void MeshGeometry::createVAO_position_uv(RenderCommandList& cmdList) {
		createVAO(cmdList, indexBuffer, &vao_position_uv, {
			{ positionBuffer, positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) },
			{ uvBuffer,       uvLocation,       2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT) }
		}, "VAO_position_uv");
	}

	void MeshGeometry::createVAO_position_normal(RenderCommandList& cmdList) {
		createVAO(cmdList, indexBuffer, &vao_position_normal, {
			{ positionBuffer, positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) },
			{ normalBuffer,   normalLocation,   3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) }
		}, "VAO_position_normal");
	}

	void MeshGeometry::createVAO_position_uv_normal(RenderCommandList& cmdList) {
		createVAO(cmdList, indexBuffer, &vao_position_uv_normal, {
			{ positionBuffer, positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) },
			{ uvBuffer,       uvLocation,       2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT) },
			{ normalBuffer,   normalLocation,   3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) }
		}, "VAO_position_uv_normal");
	}

	void MeshGeometry::createVAO_position_uv_normal_tangent_bitangent(RenderCommandList& cmdList) {
		createVAO(cmdList, indexBuffer, &vao_position_uv_normal_tangent_bitangent, {
			{ positionBuffer,  positionLocation,  3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) },
			{ uvBuffer,        uvLocation,        2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT) },
			{ normalBuffer,    normalLocation,    3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) },
			{ tangentBuffer,   tangentLocation,   3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) },
			{ bitangentBuffer, bitangentLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT) }
		}, "VAO_position_uv_normal_tangent_bitangent");
	}

}
