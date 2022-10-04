#pragma once

#include "pathos/gl_handles.h"
#include "pathos/render/render_command_list.h"
#include "pathos/overlay/display_object.h"
#include "pathos/wrapper/transform.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class DisplayObject2DProxy;

	class OverlayPass : public Noncopyable {

	public:
		virtual ~OverlayPass() = default;

		virtual void renderOverlay(
			RenderCommandList& cmdList,
			DisplayObject2DProxy* object,
			const Transform& transformAccum) = 0;

	};

}
