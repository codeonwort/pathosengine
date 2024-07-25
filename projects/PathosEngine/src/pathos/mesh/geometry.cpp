#include "pathos/mesh/geometry.h"
#include "pathos/rhi/render_device.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include <vector>

namespace pathos {

	static constexpr GLuint POSITION_LOCATION  = 0;
	static constexpr GLuint UV_LOCATION        = 1;
	static constexpr GLuint NORMAL_LOCATION    = 2;
	static constexpr GLuint TANGENT_LOCATION   = 3;
	static constexpr GLuint BITANGENT_LOCATION = 4;

	struct VAOElement {
		GLuint buffer;
		GLuint index;
		GLint  size;
		GLenum type;
		GLboolean normalized;
		GLsizei stride;
		GLintptr offset;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// MeshGeometry

	MeshGeometry::MeshGeometry() {
		localBounds = AABB{ vector3(0.0f), vector3(0.0f) };
		indexDatatype = GL_UNSIGNED_INT;
	}

	MeshGeometry::~MeshGeometry() {
		dispose();
	}

	void MeshGeometry::initializeVertexLayout(EVertexAttributes inVertexAttributes, bool useIndexBuffer /*= true*/) {
		CHECK(vertexAttributes == EVertexAttributes::None);
		vertexAttributes = inVertexAttributes;
		bUseIndexBuffer = useIndexBuffer;
	}

	void MeshGeometry::dispose() {
		disposeVAO();
		releaseBuffer(positionBuffer);
		releaseBuffer(uvBuffer);
		releaseBuffer(normalBuffer);
		releaseBuffer(tangentBuffer);
		releaseBuffer(bitangentBuffer);
		releaseBuffer(indexBuffer);
	}

	uint32 MeshGeometry::getIndexCount() const { return (uint32)indexData.size(); }
	
	// gRenderDevice->getIndexBufferPool() gaurantees 4-byte alignment
	uint32 MeshGeometry::getFirstIndex() const { return (uint32)indexBuffer.offset / (isIndex16Bit() ? 2 : 4); }

	bool MeshGeometry::isIndex16Bit() const { return indexDatatype == GL_UNSIGNED_SHORT; }

	void MeshGeometry::updatePositionData(const GLfloat* data, uint32 length) {
		CHECK(ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Position));
		CHECKF(length % 3 == 0, "Invalid length");
		const vector3* data2 = reinterpret_cast<const vector3*>(data);

		positionData.resize(length / 3);
		positionData.assign(data2, data2 + length / 3);

		bufferUploadHelper(positionBuffer, length * sizeof(GLfloat), positionData.data(), gRenderDevice->getPositionBufferPool());

		if (bCalculateLocalBounds) {
			calculateLocalBounds();
		}
	}

	void MeshGeometry::updateUVData(const GLfloat* data, uint32 length, bool bFlipY /*= false */) {
		CHECK(ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Uv));
		CHECKF(length % 2 == 0, "Invalid length");

		const vector2* data2 = reinterpret_cast<const vector2*>(data);
		uvData.resize(length / 2);
		uvData.assign(data2, data2 + length / 2);
		if (bFlipY) { for (vector2& uv : uvData) { uv.y = 1.0f - uv.y; } }

		bufferUploadHelper(uvBuffer, length * sizeof(GLfloat), uvData.data(), gRenderDevice->getVaryingBufferPool());
	}

	void MeshGeometry::updateNormalData(const GLfloat* data, uint32 length) {
		CHECK(ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Normal));
		CHECKF(length % 3 == 0, "Invalid length");

		const vector3* data2 = reinterpret_cast<const vector3*>(data);
		normalData.assign(data2, data2 + length / 3);

		bufferUploadHelper(normalBuffer, length * sizeof(GLfloat), normalData.data(), gRenderDevice->getVaryingBufferPool());
	}
	void MeshGeometry::updateNormalData(const std::vector<vector3>& inNormals)
	{
		const GLfloat* buffer = reinterpret_cast<const GLfloat*>(inNormals.data());
		uint32 length = (uint32)(inNormals.size() * 3);
		updateNormalData(buffer, length);
	}

	void MeshGeometry::updateTangentData(const GLfloat* data, uint32 length) {
		CHECK(ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Tangent));
		CHECKF(length % 4 == 0, "Invalid length");

		const vector4* data2 = reinterpret_cast<const vector4*>(data);
		tangentData.assign(data2, data2 + length / 4);

		bufferUploadHelper(tangentBuffer, length * sizeof(GLfloat), tangentData.data(), gRenderDevice->getVaryingBufferPool());
	}
	void MeshGeometry::updateBitangentData(const GLfloat* data, uint32 length) {
		CHECK(ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Bitangent));
		CHECKF(length % 3 == 0, "Invalid length");

		const vector3* data2 = reinterpret_cast<const vector3*>(data);
		bitangentData.assign(data2, data2 + length / 3);

		bufferUploadHelper(bitangentBuffer, length * sizeof(GLfloat), bitangentData.data(), gRenderDevice->getVaryingBufferPool());
	}

	void MeshGeometry::updateIndexData(const GLuint* data, uint32 length) {
		CHECK(bUseIndexBuffer);
		indexData.assign(data, data + length);

		bufferUploadHelper(indexBuffer, length * sizeof(GLuint), indexData.data(), gRenderDevice->getIndexBufferPool());
		indexDatatype = GL_UNSIGNED_INT;
	}

	void MeshGeometry::updateIndex16Data(const uint16* data, uint32 length) {
		CHECK(bUseIndexBuffer);
		// Keep shadow copy as 32-bit data
		indexData.resize(length);
		for (uint32 i = 0; i < length; ++i) indexData[i] = (GLuint)data[i];

		// Send 16-bit data to GPU
		bufferUploadHelper(indexBuffer, length * sizeof(uint16), (void*)data, gRenderDevice->getIndexBufferPool());
		indexDatatype = GL_UNSIGNED_SHORT;
	}

	// requirements: positionData and indexData should be available.
	void MeshGeometry::calculateNormals() {
		if (bUseIndexBuffer) calculateNormals_indexed(); 
		else calculateNormals_array();
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

		std::vector<GLfloat> tangents(numPos * 4, 0.0f);
		std::vector<GLfloat> bitangents(numPos * 3, 0.0f);

		for (int32 i = 0; i < numPos; ++i) {
			vector3 v = glm::normalize(accum[i]);
			bool invalid = glm::any(glm::isnan(v));
			if (invalid) {
				tangents[i * 3 + 0] = tangents[i * 3 + 1] = tangents[i * 3 + 2] = tangents[i * 3 + 3] = 0.0f;
			} else {
				tangents[i * 3 + 0] = v.x;
				tangents[i * 3 + 1] = v.y;
				tangents[i * 3 + 2] = v.z;
				tangents[i * 3 + 3] = 1.0f; // #todo-geometry: tangent w component
			}

			if (invalid) {
				bitangents[i * 3 + 0] = bitangents[i * 3 + 1] = bitangents[i * 3 + 2] = 0.0f;
			} else {
				v = glm::normalize(accum2[i]);
				bitangents[i * 3 + 0] = v.x;
				bitangents[i * 3 + 1] = v.y;
				bitangents[i * 3 + 2] = v.z;
			}
		}

		updateTangentData(tangents.data(), (uint32)tangents.size());
		updateBitangentData(bitangents.data(), (uint32)bitangents.size());
	}

	void MeshGeometry::calculateBitangentOnly() {
		CHECK(normalData.size() == tangentData.size());
		std::vector<float> Bs(tangentData.size() * 3);
		for (size_t i = 0; i < tangentData.size(); ++i) {
			vector3 B = glm::cross(normalData[i], vector3(tangentData[i]));
			Bs[3 * i + 0] = B.x;
			Bs[3 * i + 1] = B.y;
			Bs[3 * i + 2] = B.z;
		}
		updateBitangentData(Bs.data(), (uint32)Bs.size());
	}

	void MeshGeometry::calculateLocalBounds() {
		vector3 minV(0.0f), maxV(0.0f);
		if (positionData.size() > 0) {
			minV = vector3(FLT_MAX);
			maxV = vector3(-FLT_MAX);
			for (const vector3& v : positionData) {
				minV = glm::min(minV, v);
				maxV = glm::max(maxV, v);
			}
		}
		localBounds = AABB::fromMinMax(minV, maxV);
		//LOG(LogInfo, "LocalBounds");
		//LOG(LogInfo, "\tmin: %f %f %f", minV.x, minV.y, minV.z);
		//LOG(LogInfo, "\tmax: %f %f %f", maxV.x, maxV.y, maxV.z);
	}

	void MeshGeometry::bindPositionOnlyVAO(RenderCommandList& cmdList) {
		if (vaoPositionOnly == 0) {
			createPositionVAO(cmdList);
		}
		cmdList.bindVertexArray(vaoPositionOnly);
	}

	void MeshGeometry::bindFullAttributesVAO(RenderCommandList& cmdList) {
		if (vaoFullAttributes == 0) {
			createFullVAO(cmdList);
		}
		cmdList.bindVertexArray(vaoFullAttributes);
	}

	void MeshGeometry::unbindVAO(RenderCommandList& cmdList) {
		cmdList.bindVertexArray(0);
	}

	void MeshGeometry::drawPrimitive(RenderCommandList& cmdList, int32 instanceCount) {
		if (instanceCount == 1) {
			if (bUseIndexBuffer) {
				const uintptr_t indexOffsetBytes = (uintptr_t)indexBuffer.offset;
				cmdList.drawElements(GL_TRIANGLES, (GLsizei)getIndexCount(), indexDatatype, (void*)indexOffsetBytes);
			} else {
				cmdList.drawArrays(GL_TRIANGLES, 0, (GLsizei)positionData.size());
			}
		} else {
			if (bUseIndexBuffer) {
				const uintptr_t indexOffsetBytes = (uintptr_t)indexBuffer.offset;
				cmdList.drawElementsInstanced(GL_TRIANGLES, (GLsizei)getIndexCount(), indexDatatype, (void*)indexOffsetBytes, instanceCount);
			} else {
				cmdList.drawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)positionData.size(), instanceCount);
			}
		}
	}

	void MeshGeometry::bufferUploadHelper(BufferView& bufferView, uint64 requestedBytes, void* data, BufferPool* bufferPool) {
		CHECK(requestedBytes != 0);
		if (bufferView.bufferPool != nullptr) CHECK(bufferView.bufferPool == bufferPool);
		if (bufferView.offset == BufferPool::INVALID_OFFSET || bufferView.bytes != requestedBytes) {
			if (bufferView.offset != BufferPool::INVALID_OFFSET) {
				bufferPool->deallocate(bufferView.offset);
				disposeVAO(); // #todo-performance: Some cases don't need recreation of VAO
			}
			bufferView.offset = bufferPool->suballocate(requestedBytes);
			bufferView.bytes = requestedBytes;
			bufferView.bufferPool = bufferPool;
			CHECKF(bufferView.offset != BufferPool::INVALID_OFFSET, "Failed to suballocate from buffer pool");
		}
		bufferPool->writeToGPU((int64)bufferView.offset, (int64)bufferView.bytes, data);
	}

	void MeshGeometry::releaseBuffer(BufferView& bufferView) {
		if (bufferView.offset != BufferPool::INVALID_OFFSET) {
			bufferView.bufferPool->deallocate(bufferView.offset);
			bufferView.offset = BufferPool::INVALID_OFFSET;
			bufferView.bytes = 0;
			bufferView.bufferPool = nullptr;
		}
	}

	static void createVAOHelper(
		RenderCommandList& cmdList,
		GLuint indexBuffer,
		GLuint* vao,
		const std::vector<VAOElement>& descs,
		const char* debugLabel)
	{
		CHECK(*vao == 0 && indexBuffer != 0 && descs.size() > 0);
		gRenderDevice->createVertexArrays(1, vao);
		gRenderDevice->objectLabel(GL_VERTEX_ARRAY, *vao, -1, debugLabel);

		// attribindex and bindingindex are so confusing...
		for (const VAOElement& desc : descs) {
			CHECKF(desc.buffer != 0, "Null buffer is not allowed");

			cmdList.vertexArrayVertexBuffer(
				*vao,            // VAO
				desc.index,      // binding index
				desc.buffer,     // buffer name
				desc.offset,     // offset
				desc.stride      // stride
			);

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
		if (indexBuffer != 0) {
			cmdList.vertexArrayElementBuffer(*vao, indexBuffer);
		}
	}

	void MeshGeometry::createPositionVAO(RenderCommandList& cmdList) {
		const GLuint indexBufferName     = bUseIndexBuffer ? indexBuffer.bufferPool->internal_getGLName() : 0;
		const GLuint positionBufferName  = positionBuffer.bufferPool->internal_getGLName();

		std::vector<VAOElement> descs;

		//                            { bufferName,          bindingIndex,       numElements, elementType, normalized, stride,               bufferOffset                       }
		const VAOElement posDesc      { positionBufferName,  POSITION_LOCATION,  3,           GL_FLOAT,    GL_FALSE,   3 * sizeof(GL_FLOAT), (GLsizeiptr)positionBuffer.offset  };

		if (ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Position)) {
			descs.push_back(posDesc);
			createVAOHelper(cmdList, indexBufferName, &vaoPositionOnly, descs, "VAO_positionOnly");
		}
	}

	void MeshGeometry::createFullVAO(RenderCommandList& cmdList) {
		const GLuint indexBufferName     = bUseIndexBuffer ? indexBuffer.bufferPool->internal_getGLName() : 0;
		const GLuint positionBufferName  = positionBuffer.bufferPool->internal_getGLName();
		const GLuint uvBufferName        = (uvBuffer.bufferPool        != nullptr) ? uvBuffer.bufferPool->internal_getGLName()        : 0;
		const GLuint normalBufferName    = (normalBuffer.bufferPool    != nullptr) ? normalBuffer.bufferPool->internal_getGLName()    : 0;
		const GLuint tangentBufferName   = (tangentBuffer.bufferPool   != nullptr) ? tangentBuffer.bufferPool->internal_getGLName()   : 0;
		const GLuint bitangentBufferName = (bitangentBuffer.bufferPool != nullptr) ? bitangentBuffer.bufferPool->internal_getGLName() : 0;

		std::vector<VAOElement> descs;

		//                            { bufferName,          bindingIndex,       numElements, elementType, normalized, stride,               bufferOffset                       }
		const VAOElement posDesc      { positionBufferName,  POSITION_LOCATION,  3,           GL_FLOAT,    GL_FALSE,   3 * sizeof(GL_FLOAT), (GLsizeiptr)positionBuffer.offset  };
		const VAOElement uvDesc       { uvBufferName,        UV_LOCATION,        2,           GL_FLOAT,    GL_FALSE,   2 * sizeof(GL_FLOAT), (GLsizeiptr)uvBuffer.offset        };
		const VAOElement normalDesc   { normalBufferName,    NORMAL_LOCATION,    3,           GL_FLOAT,    GL_FALSE,   3 * sizeof(GL_FLOAT), (GLsizeiptr)normalBuffer.offset    };
		const VAOElement tangentDesc  { tangentBufferName,   TANGENT_LOCATION,   4,           GL_FLOAT,    GL_FALSE,   4 * sizeof(GL_FLOAT), (GLsizeiptr)tangentBuffer.offset   };
		const VAOElement bitangentDesc{ bitangentBufferName, BITANGENT_LOCATION, 3,           GL_FLOAT,    GL_FALSE,   3 * sizeof(GL_FLOAT), (GLsizeiptr)bitangentBuffer.offset };

		if (ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Position))  { descs.push_back(posDesc);       }
		if (ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Uv))        { descs.push_back(uvDesc);        }
		if (ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Normal))    { descs.push_back(normalDesc);    }
		if (ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Tangent))   { descs.push_back(tangentDesc);   }
		if (ENUM_HAS_FLAG(vertexAttributes, EVertexAttributes::Bitangent)) { descs.push_back(bitangentDesc); }

		createVAOHelper(cmdList, indexBufferName, &vaoFullAttributes, descs, "VAO_fullAttributes");
	}

	void MeshGeometry::disposeVAO() {
		std::vector<GLuint> VAO;
		if (vaoPositionOnly != 0) VAO.push_back(vaoPositionOnly);
		if (vaoFullAttributes != 0) VAO.push_back(vaoFullAttributes);
		ENQUEUE_DEFERRED_RENDER_COMMAND([VAO](RenderCommandList& cmdList) {
			gRenderDevice->deleteVertexArrays(static_cast<GLsizei>(VAO.size()), VAO.data());
		});
		vaoPositionOnly = vaoFullAttributes = 0;
	}

}
