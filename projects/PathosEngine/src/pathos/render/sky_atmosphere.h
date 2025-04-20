// Simulates the Earth's atmospheric scattering.

#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;
	class Camera;
	class Texture;
	class MeshGeometry;

	class SkyAtmospherePass : public Noncopyable {

	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderSkyAtmosphere(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera, MeshGeometry* fullscreenQuad);

	private:
		void renderToScreen(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);
		void renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene);

		void renderSkyIrradianceMap(RenderCommandList& cmdList, SceneProxy* scene); // #wip: Delete this
		void renderSkyDiffuseSH(RenderCommandList& cmdList);

		void renderSkyPrefilterMap(RenderCommandList& cmdList, SceneProxy* scene);
		void renderTransmittanceLUT(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad); // Called only once

		GLuint fbo = 0xffffffff;
		Texture* reflectionCubemap = nullptr;
		Texture* ambientCubemap = nullptr;
		UniformBuffer ubo;
		GLuint vao;
		Texture* transmittanceLUT = nullptr;

	};

}
