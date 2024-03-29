// Light shaft a.k.a. god ray

#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/scene/camera.h"

#include "badger/types/vector_types.h"

namespace pathos {

	class MeshGeometry;
	class SceneRenderer;
	struct StaticMeshProxy;

	class GodRay final {

		static constexpr uint32 GOD_RAY_SOURCE = 0;
		static constexpr uint32 GOD_RAY_RESULT = 1;

	public:
		GodRay();
		~GodRay();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		// #todo-godray: 'renderer' parameter is hack
		void renderGodRay(
			RenderCommandList& cmdList,
			SceneProxy* scene,
			Camera* camera,
			MeshGeometry* fullscreenQuad,
			SceneRenderer* renderer);

		// Quite arbitrary, not physically based
		inline void setGodRayColor(const vector3& inColor) { godRayColor = inColor; }

	private:
		void createFBO(RenderCommandList& cmdList);
		void renderSilhouette(RenderCommandList& cmdList, Camera* camera, StaticMeshProxy* mesh);

	private:
		vector3 godRayColor;

		bool destroyed = false;

		GLuint fbo[2] = { 0, 0 };

		UniformBuffer uboSilhouette;
		UniformBuffer uboLightScattering;
		// Program: gaussian blur
		GLuint fboBlur1 = 0xffffffff;
		GLuint fboBlur2 = 0xffffffff;

		GLuint vao_dummy = 0;

	};

}
