#pragma once

#include "pathos/named_object.h"
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
		void draw();

		uint32_t getIndexCount();

		void updatePositionData(GLfloat* data, uint32_t length);
		void updateIndexData(GLuint* data, uint32_t length);
		void updateUVData(GLfloat* data, uint32_t length);
		void updateNormalData(GLfloat* data, uint32_t length);
		void updateTangentData(GLfloat* data, uint32_t length);
		void updateBitangentData(GLfloat* data, uint32_t length);

		void calculateNormals();
		void calculateTangentBasis();

		void activate_position();
		void activate_position_uv();
		void activate_position_normal();
		void activate_position_uv_normal();
		void activate_position_uv_normal_tangent_bitangent();
		void deactivate();

		void activateIndexBuffer();
		void deactivateIndexBuffer();

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
		GLfloat* positionData  = nullptr;
		GLfloat* uvData        = nullptr;
		GLfloat* normalData    = nullptr;
		GLfloat* tangentData   = nullptr;
		GLfloat* bitangentData = nullptr;
		GLuint* indexData      = nullptr;

		GLuint positionBuffer  = 0;
		GLuint uvBuffer        = 0;
		GLuint normalBuffer    = 0;
		GLuint indexBuffer     = 0;
		GLuint tangentBuffer   = 0;
		GLuint bitangentBuffer = 0;

		// what's this monstrosity :(
		// Core profile forces usage of VAO, but I don't have a good idea to manage these combinations.
		GLuint vao_position                             = 0;
		GLuint vao_position_uv                          = 0;
		GLuint vao_position_normal                      = 0;
		GLuint vao_position_uv_normal                   = 0;
		GLuint vao_position_uv_normal_tangent_bitangent = 0;

		// they are array lengths, not actual counts!
		uint32_t positionCount = 0;
		uint32_t uvCount       = 0;
		uint32_t normalCount   = 0;
		uint32_t indexCount    = 0;

		bool drawArraysMode; // use glDrawArrays() if true. use glDrawElements() if false. (default: false)
	};

}
