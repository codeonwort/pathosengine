#include "render_command_list.h"
#include "render_commands.h"

namespace pathos {

	void RenderCommandList::reset()
	{
		commands_alloc.clear();
		commands.clear();
	}

	RenderCommandBase* RenderCommandList::getNextPacket()
	{
		RenderCommandBase* packet = (RenderCommandBase*)commands_alloc.alloc(sizeof(RenderCommandPacketUnion));
		commands.push_back(packet);
		return packet;
	}

}
