#pragma once

#include "overlaypass_base.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class DisplayObject2DProxy;

	// standard render pass that uses color and texture
	class OverlayPass_Standard : public OverlayPass {

	public:
		OverlayPass_Standard();
		~OverlayPass_Standard() = default;

		virtual void renderOverlay(
			RenderCommandList& cmdList,
			DisplayObject2DProxy* object,
			const Transform& transformAccum) override;

		inline void setColor(const vector4& inColor) { rgba = inColor; }

	private:
		UniformBuffer ubo;
		vector4 rgba;

	};

}
