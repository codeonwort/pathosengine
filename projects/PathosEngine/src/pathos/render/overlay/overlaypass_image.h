#pragma once

#include "overlaypass_base.h"
#include "pathos/rhi/uniform_buffer.h"

namespace pathos {

	class DisplayObject2DProxy;

	// standard render pass that uses color and texture
	class OverlayPass_Image : public OverlayPass {

	public:
		OverlayPass_Image();

		virtual void renderOverlay(
			RenderCommandList& cmdList,
			DisplayObject2DProxy* object,
			const Transform& transformAccum) override;

		inline void setImageTexture(GLuint texture) { textureName = texture; }

	private:
		UniformBuffer ubo;
		GLuint textureName = 0;

	};

}
