#pragma once

#include "post_process.h"
#include "pathos/shader/uniform_buffer.h"

// AMD FSR1 (https://gpuopen.com/fidelityfx-superresolution/)

namespace pathos {

	class FSR1 final : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		UniformBuffer ubo;

	};

}
