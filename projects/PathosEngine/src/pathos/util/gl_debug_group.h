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

}

// Assumes 'RenderCommandList& cmdList' is defined in the caller
#define SCOPED_DRAW_EVENT(EventName) pathos::DebugGroupMarker DebugGroup_##EventName(&cmdList, #EventName);
