#pragma once

#include "pathos/named_object.h"
#include "pathos/render/render_command_list.h"
#include "gl_core.h"

#include "badger/types/vector_types.h"
#include "badger/math/aabb.h"

namespace pathos {

	class MeshGeometry : public NamedObject {

	public:
		MeshGeometry();
		virtual ~MeshGeometry();

		MeshGeometry(const MeshGeometry&)            = delete;
		MeshGeometry& operator=(const MeshGeometry&) = delete;

		void setDrawArraysMode(bool value) { drawArraysMode = value; }

		// #todo-draw-prim: Remove this and call cmdList.drawElements() directly
		void drawPrimitive(RenderCommandList& cmdList);

		inline vector3 getPosition(uint32 index) const { return positionData[index]; }
		uint32 getIndexCount() const;

		void updateNormalData(const std::vector<vector3>& inNormals);

		void updatePositionData(const GLfloat* data, uint32 length);
		void updateIndexData(const GLuint* data, uint32 length);
		void updateUVData(const GLfloat* data, uint32 length);
		void updateNormalData(const GLfloat* data, uint32 length);
		void updateTangentData(const GLfloat* data, uint32 length);
		void updateBitangentData(const GLfloat* data, uint32 length);

		void calculateNormals();
		void calculateTangentBasis();

		void calculateLocalBounds();
		inline const AABB& getLocalBounds() const { return localBounds; }

		void activate_position(RenderCommandList& cmdList);
		void activate_position_uv(RenderCommandList& cmdList);
		void activate_position_normal(RenderCommandList& cmdList);
		void activate_position_uv_normal(RenderCommandList& cmdList);
		void activate_position_uv_normal_tangent_bitangent(RenderCommandList& cmdList);
		void deactivate(RenderCommandList& cmdList);

		void activateIndexBuffer(RenderCommandList& cmdList);
		void deactivateIndexBuffer(RenderCommandList& cmdList);

		void dispose();

	private:
		void createVAO_position(RenderCommandList& cmdList);
		void createVAO_position_uv(RenderCommandList& cmdList);
		void createVAO_position_normal(RenderCommandList& cmdList);
		void createVAO_position_uv_normal(RenderCommandList& cmdList);
		void createVAO_position_uv_normal_tangent_bitangent(RenderCommandList& cmdList);

		void calculateNormals_array();
		void calculateNormals_indexed();

	public:
		bool bCalculateLocalBounds = true;

	private:
		std::vector<vector3> positionData;
		std::vector<vector2> uvData;
		std::vector<vector3> normalData;
		std::vector<vector3> tangentData;
		std::vector<vector3> bitangentData;

		// #todo-vao: Support both GL_UNSIGNED_SHORT (16bit) and GL_UNSIGNED_INT (32bit)
		std::vector<GLuint>  indexData;

		AABB localBounds;

		// VBOs
		GLuint positionBuffer  = 0;
		GLuint uvBuffer        = 0;
		GLuint normalBuffer    = 0;
		GLuint tangentBuffer   = 0;
		GLuint bitangentBuffer = 0;
		GLuint indexBuffer     = 0;

		// VAOs
		GLuint vao_position                             = 0;
		GLuint vao_position_uv                          = 0;
		GLuint vao_position_normal                      = 0;
		GLuint vao_position_uv_normal                   = 0;
		GLuint vao_position_uv_normal_tangent_bitangent = 0;

		bool drawArraysMode; // use glDrawArrays() if true. use glDrawElements() if false. (default: false)
	};

}
