// Light shaft a.k.a. god ray

#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/scene/camera.h"

#include "badger/types/vector_types.h"
#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;
	class MeshGeometry;
	class SceneRenderer;
	struct StaticMeshProxy;

	class GodRay final : public Noncopyable {

	public:
		GodRay();
		~GodRay();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		// Render god ray to a separate texture.
		// #todo-godray: 'renderer' parameter is hack
		void renderGodRay(
			RenderCommandList& cmdList,
			SceneProxy* scene,
			Camera* camera,
			SceneRenderer* renderer);

		// Blend god ray with sceneColor.
		void renderGodRayPost(RenderCommandList& cmdList, SceneProxy* scene);

		// Quite arbitrary, not physically based
		inline void setGodRayColor(const vector3& inColor) { godRayColor = inColor; }

	private:
		void createFBO(RenderCommandList& cmdList);
		void renderSilhouette(RenderCommandList& cmdList, Camera* camera, StaticMeshProxy* mesh);
		bool isPassEnabled(SceneProxy* scene) const;

	private:
		vector3 godRayColor;

		bool destroyed = false;

		GLuint fboSilhouette = 0xffffffff;
		GLuint fboLight = 0xffffffff;
		GLuint fboBlurH = 0xffffffff;
		GLuint fboBlurV = 0xffffffff;
		GLuint fboPost = 0xffffffff;

		UniformBuffer uboSilhouette;
		UniformBuffer uboLightScattering;

	};

}
