#include "gl_debug_group.h"

namespace pathos {

	DebugGroupMarker::DebugGroupMarker(class RenderCommandList* cmdList, const char* eventName)
	{
		uint32_t id = COMPILE_TIME_CRC32_STR(eventName);
		cmdList->pushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, -1, eventName);
		command_list = cmdList;
	}

	DebugGroupMarker::~DebugGroupMarker()
	{
		command_list->popDebugGroup();
	}

}
