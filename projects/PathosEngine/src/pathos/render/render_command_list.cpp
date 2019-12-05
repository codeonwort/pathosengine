#include "render_command_list.h"
#include "render_commands.h"

namespace pathos {

	void RenderCommandList::clearAllCommands()
	{
		commands_alloc.clear();
		commands.clear();
	}

	void RenderCommandList::executeAllCommands()
	{
		for (RenderCommandBase* __restrict cmd : commands)
		{
			cmd->pfn_execute((RenderCommandBase*)cmd);
		}
	}

	RenderCommandBase* RenderCommandList::getNextPacket()
	{
		RenderCommandBase* packet = (RenderCommandBase*)commands_alloc.alloc(sizeof(RenderCommandPacketUnion));
		commands.push_back(packet);
		return packet;
	}

}
