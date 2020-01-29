#pragma once

#include <stdint.h>
#include <vector>
#include "gl_core.h"

#include "light.h"
#include "shadow_omni.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include "pathos/mesh/geometry.h"

// TODO: move ShadowMap to shadow_directional.h
//       make this header includes shadow_omni.h and shadow_directional.h

namespace pathos {

	// #todo-shadow: This is deprecated. @see DirectionalShadowMap
	class ShadowMap {

	public:
		// add shadow test logic to material pass
		static void addShaderCode(VertexShaderSource&, FragmentShaderSource&, unsigned int maxLights);

	public:
		ShadowMap(unsigned int maxDirectionalLights);
		ShadowMap(const ShadowMap& other) = delete;
		ShadowMap(ShadowMap&& other) = delete;
		virtual ~ShadowMap();

		void clearLightDepths(uint32_t numLights);
		void renderLightDepth(RenderCommandList& cmdList, uint32 lightIndex, DirectionalLight* light, MeshGeometry* mesh, const glm::mat4& modelMatrix);

		inline void setProjection(glm::mat4& newProjection) { projection = newProjection; }

		// add shadow algorithm to the shadow castee's program
		void activate(GLuint materialPassProgram, const vector<DirectionalLight*>& lights, unsigned int textureBinding, const glm::mat4& modelMatrix);
		void deactivate(GLuint materialPassProgram, unsigned int textureBinding);

		inline const GLuint getDebugTexture(uint32_t index) const { return static_cast<GLuint>(depthTextures[index]); }

	private:
		GLuint fbo;
		GLsizei width, height; // dimension of depth textures

		unsigned int maxLights;
		vector<GLuint> depthTextures;

		GLuint program;
		GLint uniform_depthMVP = -1;
		vector<GLint> textureBindings;

		glm::mat4 view, projection; // light space transform
	};

}