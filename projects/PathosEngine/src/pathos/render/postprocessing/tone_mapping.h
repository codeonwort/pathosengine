#pragma once

#include "post_process.h"
#include "pathos/rhi/uniform_buffer.h"

namespace pathos {

	class ToneMapping final : public PostProcess {
		
	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad) override;

		// Call this before renderPostProcess()
		inline void setParameters(bool useAutoExposure, bool applyBloom) {
			bUseAutoExposure = useAutoExposure;
			bApplyBloom = applyBloom;
		}

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;
		bool bUseAutoExposure = true;
		bool bApplyBloom = true;

	};

}
