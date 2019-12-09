#pragma once

#include "overlaypass_base.h"

namespace pathos {

	// standard render pass that uses color and texture
	class OverlayPass_Standard : public OverlayPass {

	public:
		OverlayPass_Standard();
		~OverlayPass_Standard();

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