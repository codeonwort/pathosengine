#include "render_command_list.h"
#include "render_commands.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	void RenderCommandList::clearAllCommands()
	{
		commands_alloc.clear();
		parameters_alloc.clear();
		commands.clear();
	}

	void RenderCommandList::executeAllCommands()
	{
		for (RenderCommandBase* __restrict cmd : commands)
		{
			cmd->pfn_execute((RenderCommandBase*)cmd);
		}
	}

	void RenderCommandList::flushAllCommands()
	{
		executeAllCommands();
		clearAllCommands();
	}

	void RenderCommandList::registerHook(std::function<void(void*)> hook, void* argument)
	{
		RenderCommand_registerHook* __restrict packet = (RenderCommand_registerHook*)getNextPacket();
		packet->pfn_execute = PFN_EXECUTE(RenderCommand_registerHook::execute);
		packet->hook = hook;
		packet->argument = argument;
	}

	RenderCommandBase* RenderCommandList::getNextPacket()
	{
		RenderCommandBase* packet = (RenderCommandBase*)commands_alloc.alloc(sizeof(RenderCommandPacketUnion));
		CHECK(packet != nullptr);
		commands.push_back(packet);
		return packet;
	}

}
