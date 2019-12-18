#pragma once

#include "post_process.h"

namespace pathos {

	class FXAA final : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		GLuint program = 0xffffffff;
		GLuint fbo = 0;

		// pixel shader parameters
		GLint uniform_inv_size;
		GLint fxaaQualityRcpFrame;
		GLint fxaaConsoleRcpFrameOpt;
		GLint fxaaConsoleRcpFrameOpt2;
		GLint fxaaConsole360RcpFrameOpt2;
		GLint fxaaQualitySubpix;
		GLint fxaaQualityEdgeThreshold;
		GLint fxaaQualityEdgeThresholdMin;
		GLint fxaaConsoleEdgeSharpness;
		GLint fxaaConsoleEdgeThreshold;
		GLint fxaaConsoleEdgeThresholdMin;
		GLint fxaaConsole360ConstDir;

	};

}
