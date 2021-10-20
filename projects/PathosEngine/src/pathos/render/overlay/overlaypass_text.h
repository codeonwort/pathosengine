#pragma once

#include "overlaypass_base.h"
#include "pathos/shader/uniform_buffer.h"

#include "badger/types/vector_types.h"

namespace pathos {

	class OverlayPass_Text : public OverlayPass {

	public:
		OverlayPass_Text();
		~OverlayPass_Text() = default;

		virtual void renderOverlay(RenderCommandList& cmdList, DisplayObject2D* object, const Transform& transformAccum) override;

		inline void setColor(const vector4& inColor) { color = inColor; }

	private:
		UniformBuffer ubo;
		vector4 color;

	};

}
