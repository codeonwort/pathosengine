#pragma once

#include "pathos/named_object.h"
#include "pathos/render/render_command_list.h"
#include "gl_core.h"

#include <stdint.h>

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

		uint32_t getIndexCount() const;

		void updatePositionData(GLfloat* data, uint32_t length);
		void updateIndexData(GLuint* data, uint32_t length);
		void updateUVData(GLfloat* data, uint32_t length);
		void updateNormalData(GLfloat* data, uint32_t length);
		void updateTangentData(GLfloat* data, uint32_t length);
		void updateBitangentData(GLfloat* data, uint32_t length);

		void calculateNormals();
		void calculateTangentBasis();

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
		void createVAO_position();
		void createVAO_position_uv();
		void createVAO_position_normal();
		void createVAO_position_uv_normal();
		void createVAO_position_uv_normal_tangent_bitangent();

		void calculateNormals_array();
		void calculateNormals_indexed();

	private:
		std::vector<GLfloat> positionData;
		std::vector<GLfloat> uvData;
		std::vector<GLfloat> normalData;
		std::vector<GLfloat> tangentData;
		std::vector<GLfloat> bitangentData;
		std::vector<GLuint>  indexData;

		GLuint positionBuffer  = 0;
		GLuint uvBuffer        = 0;
		GLuint normalBuffer    = 0;
		GLuint indexBuffer     = 0;
		GLuint tangentBuffer   = 0;
		GLuint bitangentBuffer = 0;

		// #todo-shader: Core profile forces usage of VAO, but I don't have a good idea to manage these combinations.
		GLuint vao_position                             = 0;
		GLuint vao_position_uv                          = 0;
		GLuint vao_position_normal                      = 0;
		GLuint vao_position_uv_normal                   = 0;
		GLuint vao_position_uv_normal_tangent_bitangent = 0;

		// Array lengths, not actual counts!
		uint32 positionBufferBytes = 0;
		uint32 uvBufferBytes       = 0;
		uint32 normalBufferBytes   = 0;
		uint32 indexCount          = 0;

		bool drawArraysMode; // use glDrawArrays() if true. use glDrawElements() if false. (default: false)
	};

}
