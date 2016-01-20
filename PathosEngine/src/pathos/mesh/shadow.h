#pragma once

#include <GL/glew.h>
#include <pathos/light/light.h>
#include <pathos/camera/camera.h>
#include <pathos/mesh/shader.h>
#include <pathos/mesh/geometry.h>

namespace pathos {

	class ShadowMethod {
	protected:
		MeshGeometry* modelGeometry;
		glm::mat4 modelMatrix;
	public:
		static std::vector<ShadowMethod*> instances;
		static void clearShadowTextures();

		inline void setTarget(MeshGeometry* G, glm::mat4& M) {
			modelGeometry = G;
			modelMatrix = M;
		}
		virtual void clearTexture() = 0;
		virtual void activate(GLuint materialPassProgram) = 0;
		virtual void renderDepth() = 0;
		virtual void deactivate() = 0;
		virtual void addShaderCode(VertexShaderCompiler&, FragmentShaderCompiler&) = 0;
	};

	// shadow mapping (by directional light)
	class ShadowMap : public ShadowMethod {
	private:
		DirectionalLight* light;
		Camera* camera;

		GLsizei width, height;
		GLuint fbo, shadowTexture, debugTexture;
		GLuint program;
		glm::mat4 depthMVP;

		glm::vec4 vertices[8];
		glm::mat4 calculateAABB(glm::mat4& lightView);

	public:
		ShadowMap(DirectionalLight*, Camera*);
		virtual ~ShadowMap();

		virtual void clearTexture();
		virtual void activate(GLuint materialPassProgram);
		virtual void renderDepth();
		virtual void deactivate();
		virtual void addShaderCode(VertexShaderCompiler&, FragmentShaderCompiler&);

		inline GLuint getDebugTexture() { return debugTexture; }
	};

	// shadow mapping (omnidirectional, by point light)
	class OmnidirectionalShadow : public ShadowMethod {
	private:
		PointLight* light;
		Camera* camera;
		GLuint fbo[6], shadowTextures[6], debugTextures[6];
		GLuint program;
		GLsizei width, height;
		glm::mat4 depthMVP;
	public:
		OmnidirectionalShadow(PointLight* light, Camera* camera);
		virtual void activate(GLuint materialPassProgram);
		virtual void renderDepth();
		virtual void deactivate();
	};

}