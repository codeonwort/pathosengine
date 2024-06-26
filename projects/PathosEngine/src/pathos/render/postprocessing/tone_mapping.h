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
		inline void setParameters(
			bool inUseAutoExposure,
			uint32 inLuminanceTargetMip,
			bool inLuminanceLogScale,
			float inExposureOverride,
			float inExposureCompensation,
			bool inApplyBloom)
		{
			bUseAutoExposure = inUseAutoExposure;
			luminanceTargetMip = inLuminanceTargetMip;
			bLuminanceLogScale = inLuminanceLogScale;
			exposureOverride = inExposureOverride;
			exposureCompensation = inExposureCompensation;
			bApplyBloom = inApplyBloom;
		}

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;

		bool bUseAutoExposure = true;
		uint32 luminanceTargetMip = 0;
		bool bLuminanceLogScale = false;
		float exposureOverride = 0.0f;
		float exposureCompensation = 0.0f;

		bool bApplyBloom = true;

	};

}
