#pragma once

#include "sky.h"
#include "pathos/mesh/geometry_primitive.h"
#include "gl_core.h"

namespace pathos {

	class Skybox : public SkyActor {

	public:
		void initialize(GLuint textureID);
		void setLOD(float inLOD);

		void render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera) override;

	protected:
		virtual void onDestroy() override;

	private:
		void createShader();

		GLuint program = 0;
		GLuint textureID = 0;
		GLint uniform_transform = -1;
		GLint uniform_lod = -1;

		CubeGeometry* cube = nullptr;
		float lod = 0.0f;

	};

}
