#pragma once

#include "light.h"
#include "shadow_omni.h"
#include "pathos/camera/camera.h"
#include "pathos/render/shader.h"
#include "pathos/mesh/geometry.h"
#include <GL/glew.h>
#include <vector>

// TODO: move ShadowMap to shadow_directional.h
//       make this header includes shadow_omni.h and shadow_directional.h

namespace pathos {

	// shadow mapping by directional light
	class ShadowMap {

	public:
		// add shadow test logic to material pass
		static void addShaderCode(VertexShaderSource&, FragmentShaderSource&, unsigned int maxLights);

	public:
		ShadowMap(unsigned int maxDirectionalLights);
		ShadowMap(const ShadowMap& other) = delete;
		ShadowMap(ShadowMap&& other) = delete;
		virtual ~ShadowMap();

		void clearLightDepths(unsigned int numLights);
		void renderLightDepth(unsigned int lightIndex, DirectionalLight* light, MeshGeometry*, const glm::mat4& modelMatrix);

		inline void setProjection(glm::mat4& newProjection) { projection = newProjection; }

		// add shadow algorithm to the shadow castee's program
		void activate(GLuint materialPassProgram, const vector<DirectionalLight*>& lights, unsigned int textureBinding, const glm::mat4& modelMatrix);
		void deactivate(GLuint materialPassProgram, unsigned int textureBinding);

		inline const GLuint getDebugTexture(unsigned int index) const { return depthTextures[index]; }

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