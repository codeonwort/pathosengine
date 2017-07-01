#pragma once

#include "overlaypass_base.h"

namespace pathos {

	// standard render pass that uses color and texture
	class OverlayPass_Standard : public OverlayPass {

	private:
		static constexpr unsigned int TEXTURE_UNIT = 0;

	public:
		OverlayPass_Standard();

		virtual void render(DisplayObject2D* object, const Transform& transformAccum) override;

	protected:
		virtual void createProgram() override;

	private:
		GLint uniform_transform = -1;

	};

}