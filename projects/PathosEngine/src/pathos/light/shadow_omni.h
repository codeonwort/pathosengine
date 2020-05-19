#pragma once

#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include "pathos/mesh/geometry.h"
#include "gl_core.h"
#include <vector>

namespace pathos {

	struct PointLightProxy;

	// shadow mapping (omnidirectional, by point light)
	class OmnidirectionalShadow {

	public:
		static void addShaderCode(VertexShaderSource&, FragmentShaderSource&, unsigned int maxLights);

	public:
		OmnidirectionalShadow(unsigned int maxPointLights);
		OmnidirectionalShadow(const OmnidirectionalShadow& other) = delete;
		OmnidirectionalShadow(OmnidirectionalShadow&& rhs) = delete;
		virtual ~OmnidirectionalShadow();

		void clearLightDepths(unsigned int numLights);
		void renderLightDepth(RenderCommandList& cmdList, uint32 lightIndex, PointLightProxy* light, MeshGeometry* mesh, const glm::mat4& modelMatrix);

		// add shadow algorithm to the shadow castee's program
		void activate(GLuint materialPassProgram, const vector<PointLightProxy*>& lights, unsigned int textureBinding, const glm::mat4& modelMatrix);
		void deactivate(GLuint materialPassProgram, unsigned int textureBinding);

		inline const GLuint getDebugTexture(unsigned int index) { return depthTextures[index]; }
		inline const GLfloat getLightNearZ() { return lightNearZ; }
		inline const GLfloat getLightFarZ() { return lightFarZ; }

	private:
		GLuint fbo;
		GLsizei width, height; // dimension of depth textures

		unsigned int maxLights;
		vector<GLuint> depthTextures;
		GLfloat lightNearZ, lightFarZ;

		GLuint program;
		GLint uniform_depthMVP = -1;
		vector<GLint> textureBindings;
	};

}