#pragma once

#include "pathos/overlay/display_object.h"
#include "pathos/wrapper/transform.h"

#include <GL/glew.h>

namespace pathos {

	class OverlayPass {

	public:
		virtual ~OverlayPass();

		virtual void render(DisplayObject2D* object, const Transform& transformAccum) = 0;

	protected:
		GLuint program = 0;

		virtual void createProgram() = 0;

	};

}