#pragma once

#include "badger/types/string_hash.h"

#include "pathos/render/render_command_list.h"

namespace pathos {

	struct DebugGroupMarker {
		DebugGroupMarker(class RenderCommandList* cmdList, const char* eventName);
		~DebugGroupMarker();
	private:
		class RenderCommandList* command_list;
	};

	struct DebugGroupMarker_DEPRECATED {
		DebugGroupMarker_DEPRECATED(const char* eventName) {
			uint32_t id = COMPILE_TIME_CRC32_STR(eventName);
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, -1, eventName);
		}
		~DebugGroupMarker_DEPRECATED() {
			glPopDebugGroup();
		}
	};

}

// Assumes 'RenderCommandList& cmdList' is defined in the caller
#define SCOPED_DRAW_EVENT(EventName) pathos::DebugGroupMarker DebugGroup_##EventName(&cmdList, #EventName);

// #todo: Remove this
#define DEPRECATED_SCOPED_DRAW_EVENT(EventName) pathos::DebugGroupMarker_DEPRECATED DebugGroup_##EventName(#EventName);
