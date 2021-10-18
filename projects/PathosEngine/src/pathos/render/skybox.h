#pragma once

#include "sky.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/shader/uniform_buffer.h"
#include "gl_core.h"

namespace pathos {

	class Skybox : public SkyActor {

	public:
		void initialize(GLuint textureID);
		void setLOD(float inLOD);

		void render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera) override;

		virtual void onSpawn() override;
		virtual void onDestroy() override;

	private:
		UniformBuffer ubo;
		CubeGeometry* cube = nullptr;
		GLuint textureID = 0;
		float lod = 0.0f;
	};

}
