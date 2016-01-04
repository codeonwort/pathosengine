#pragma once

#include <GL/glew.h>
#include <pathos/light/light.h>
#include <pathos/camera/camera.h>
#include <pathos/mesh/geometry.h>

namespace pathos {

	class ShadowMap {
	private:
		DirectionalLight* light;
		Camera* camera;

		GLsizei width, height;
		GLuint fbo, shadowTexture, debugTexture, depthBuffer;
		GLuint program;
		MeshGeometry* geometry;
		glm::mat4 depthMVP;

		glm::vec4 vertices[8];
		glm::mat4 calculateAABB(glm::mat4& lightView);

	public:
		static std::vector<ShadowMap*> instances;
		static void clearShadowTextures();

		ShadowMap(DirectionalLight*, Camera*);
		virtual ~ShadowMap();

		void setGeometry(MeshGeometry*);
		void clearTexture();
		void activate(const glm::mat4 & modelMatrix);
		void renderDepth();
		void deactivate();

		inline GLuint getTexture() { return shadowTexture; }
		inline GLuint getDebugTexture() { return debugTexture; }
		inline const glm::mat4& getDepthMVP() { return depthMVP; }
		inline const GLfloat* getLight() { return light->getDirection(); }
	};

}