#pragma once

#include "post_process.h"
#include "pathos/rhi/uniform_buffer.h"

// AMD FSR1 (https://gpuopen.com/fidelityfx-superresolution/)

namespace pathos {

	class FSR1 final : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad) override;

	private:
		UniformBuffer ubo;

	};

}
