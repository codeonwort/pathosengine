#pragma once

#include "pathos/core_minimal.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/shader_storage_buffer.h"

namespace pathos {

	class SceneProxy;
	class Camera;
	class SphereGeometry;

	class VisualizeLightProbePass {

	public:
		VisualizeLightProbePass();
		~VisualizeLightProbePass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);
		void render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		SphereGeometry* sphereGeom = nullptr;
		GLuint fbo = 0;
		UniformBuffer ubo;
		ShaderStorageBuffer ssbo0; // For irradiance probes
		ShaderStorageBuffer ssbo1; // For radiance probes

	};

}
