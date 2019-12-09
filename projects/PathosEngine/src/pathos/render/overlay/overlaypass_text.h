#pragma once

#include "overlaypass_base.h"

namespace pathos {

	class OverlayPass_Text : public OverlayPass {

	public:
		OverlayPass_Text();
		~OverlayPass_Text();

		virtual void renderOverlay(RenderCommandList& cmdList, DisplayObject2D* object, const Transform& transformAccum) override;

		void setUniform_color(float rgba[4]);

	protected:
		virtual void createProgram() override;

	private:
		float rgba[4];
		GLint uniform_transform;
		GLint uniform_color;

	};

}
