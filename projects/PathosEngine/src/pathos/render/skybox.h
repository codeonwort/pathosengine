#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/mesh/geometry_primitive.h"

#include "badger/types/matrix_types.h"

namespace pathos {

	class SceneProxy;
	class MeshGeometry;

	class SkyboxPass : public Noncopyable {

	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderSkybox(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void renderSkyboxToScreen(RenderCommandList& cmdList, SceneProxy* scene);
		void renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene, int32 faceBegin, int32 faceEnd);
		void renderSkyMaterialToCubemap(RenderCommandList& cmdList, SceneProxy* scene, int32 faceBegin, int32 faceEnd);
		void generateCubemapMips(RenderCommandList& cmdList);
		void computeDiffuseSH(RenderCommandList& cmdList);
		void filterSpecular(RenderCommandList& cmdList);

		GLuint fbo = 0xffffffff;
		GLuint fboCube = 0xffffffff;

		MeshGeometry* cubeGeometry = nullptr;
		matrix4 cubeTransforms[6];

		Texture* reflectionCubemap = nullptr;
		Texture* ambientCubemap = nullptr;

		UniformBuffer ubo;
		UniformBuffer uboPerObject;

	};

}
