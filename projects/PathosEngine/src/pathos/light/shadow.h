#pragma once

#include "light.h"
#include "shadow_omni.h"
#include "pathos/camera/camera.h"
#include "pathos/render/shader.h"
#include "pathos/mesh/geometry.h"
#include <GL/glew.h>
#include <vector>

namespace pathos {

	class ShadowMethod {

	protected:
		MeshGeometry* modelGeometry;
		glm::mat4 modelMatrix;

	public:
		//static std::vector<ShadowMethod*> instances;
		//static void clearShadowTextures();

		inline void setTarget(MeshGeometry* G, glm::mat4& M) {
			modelGeometry = G;
			modelMatrix = M;
		}
		virtual void clearTexture() = 0;
		virtual void activate(GLuint materialPassProgram) = 0;
		//virtual void renderToDepthMap() = 0;
		virtual void deactivate() = 0;
		virtual void addShaderCode(VertexShaderSource&, FragmentShaderSource&) = 0;

	};

	// shadow mapping (by directional light)
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

		inline void setProjection(glm::mat4& projection) { this->projection = projection; }

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


		// TODO: 이게 maxLights만큼 있어야 함 ㅅㅄㅄㅄㅄㅄㅄㅄㅂ 아 난 개병신이다!!!! 으아악강아아아악!!!!!
		glm::mat4 view, projection; // light space transform
		// deprecated
		//DirectionalLight* light;
		//Camera* camera;
		//GLuint shadowTexture, debugTexture;
		//glm::vec4 vertices[8];
		//glm::mat4 calculateAABB(glm::mat4& lightView);
	};

}