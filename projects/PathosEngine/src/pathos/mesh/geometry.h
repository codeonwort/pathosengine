#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/buffer.h"
#include "pathos/rhi/render_command_list.h"

#include "badger/types/noncopyable.h"
#include "badger/types/vector_types.h"
#include "badger/types/enum.h"
#include "badger/math/aabb.h"

namespace pathos {

	class MeshGeometry : public Noncopyable {

	public:
		enum class EVertexAttributes : uint32 {
			None      = 0,
			Position  = 1 << 0,
			Uv        = 1 << 1,
			Normal    = 1 << 2,
			Tangent   = 1 << 3,
			Bitangent = 1 << 4,
			All       = Position | Uv | Normal | Tangent | Bitangent,
		};

	private:
		struct BufferView {
			uint64 offset = BufferPool::INVALID_OFFSET;
			uint64 bytes = 0;
			BufferPool* bufferPool = nullptr;
		};

	// API for game thread
	public:
		MeshGeometry();
		virtual ~MeshGeometry();

		void initializeVertexLayout(EVertexAttributes inVertexAttributes, bool useIndexBuffer = true);

		void dispose();

		void updateNormalData(const std::vector<vector3>& inNormals);

		// #note-geometry:
		// - GL uses GLsizeiptr(int64), but I'm restricting to uint32.
		// - 'length' can be confusing. It means the number of elements, not total bytes.

		void updatePositionData(const GLfloat* data, uint32 length);                // data: flattened array of vec3, length: the number of floats
		void updateUVData(const GLfloat* data, uint32 length, bool bFlipY = false); // data: flattened array of vec2, length: the number of floats
		void updateNormalData(const GLfloat* data, uint32 length);                  // data: flattened array of vec3, length: the number of floats
		void updateTangentData(const GLfloat* data, uint32 length);                 // data: flattened array of vec4, length: the number of floats
		void updateBitangentData(const GLfloat* data, uint32 length);               // data: flattened array of vec3, length: the number of floats

		void updateIndexData(const GLuint* data, uint32 length);                    // data: flattened array of uvec3, length: the number of uints
		void updateIndex16Data(const uint16* data, uint32 length);                  // data: flattened array of ushort3, length: the number of shorts

		void calculateNormals();
		void calculateTangentBasis();
		void calculateBitangentOnly();
		void calculateLocalBounds();

		inline vector3 getPosition(uint32 index) const { return positionData[index]; }
		inline const AABB& getLocalBounds() const { return localBounds; }

		bool isUsingPositionBufferPool() const;
		uint64 getFirstVertex() const;
		uint32 getIndexCount() const;
		uint32 getFirstIndex() const;
		bool isIndex16Bit() const;

		bool bCalculateLocalBounds = true;

	// API for render thread
	public:
		void bindPositionOnlyVAO(RenderCommandList& cmdList); // position + index if any
		void bindFullAttributesVAO(RenderCommandList& cmdList); // position + varyings + index if any
		void unbindVAO(RenderCommandList& cmdList);
		void drawPrimitive(RenderCommandList& cmdList, int32 instanceCount = 1);

	private:
		void calculateNormals_array();
		void calculateNormals_indexed();

		void createPositionVAO(RenderCommandList& cmdList);
		void createFullVAO(RenderCommandList& cmdList);
		void disposeVAO();

		void bufferUploadHelper(BufferView& bufferView, uint64 requestedBytes, void* data, BufferPool* bufferPool);
		void releaseBuffer(BufferView& bufferView);
	
	// For CPU
	private:
		std::vector<vector3> positionData;
		std::vector<vector2> uvData;
		std::vector<vector3> normalData;
		std::vector<vector4> tangentData;
		std::vector<vector3> bitangentData;
		
		std::vector<GLuint>  indexData;
		GLenum indexDatatype; // GL_UNSIGNED_SHORT (16bit) or GL_UNSIGNED_INT (32bit)

		AABB localBounds;

	// For GPU
	private:
		// Vertex layout
		EVertexAttributes vertexAttributes = EVertexAttributes::None;
		bool bUseIndexBuffer = true;
		GLuint vaoPositionOnly = 0;
		GLuint vaoFullAttributes = 0;

		// Vertex buffer, position only (suballocated from global position buffer pool)
		BufferView positionBuffer;
		// Vertex buffer, non-position (suballocated from global varying buffer pool)
		BufferView uvBuffer;
		BufferView normalBuffer;
		BufferView tangentBuffer;
		BufferView bitangentBuffer; // #todo-geometry: Remove and derive within shader
		// Index buffer (suballocated from global index buffer pool)
		BufferView indexBuffer;
	};

	ENUM_CLASS_FLAGS(MeshGeometry::EVertexAttributes);

}
