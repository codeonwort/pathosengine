#include "render_command_list.h"
#include "render_commands.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	const uint32 RenderCommandList::RENDER_COMMAND_LIST_MAX_MEMORY = 32 * 1024 * 1024; // 32 MB
	const uint32 RenderCommandList::COMMAND_PARAMETERS_MAX_MEMORY = 16 * 1024 * 1024; // 16 MB

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

	// #todo-cmd-list: Support nested flush?
	void RenderCommandList::flushAllCommands()
	{
		++flushDepth;
		if (flushDepth == 1) {
			executeAllCommands();
			clearAllCommands();
		} else {
			CHECKF(0, "Can't nest flushAllCommands()");
		}
		--flushDepth;
	}

	void RenderCommandList::registerHook(std::function<void(void*)> hook, void* argument, uint64 argumentBytes)
	{
		RenderCommand_registerHook* __restrict packet = (RenderCommand_registerHook*)getNextPacket();
		memset(packet, 0, sizeof(RenderCommandPacketUnion));
		packet->pfn_execute = PFN_EXECUTE(RenderCommand_registerHook::execute);
		packet->hook = hook;
		packet->argument = (argumentBytes == 0) ? nullptr : storeParameter(argumentBytes, argument);
	}

	RenderCommandBase* RenderCommandList::getNextPacket()
	{
		RenderCommandBase* packet = (RenderCommandBase*)commands_alloc.alloc(sizeof(RenderCommandPacketUnion));
		CHECKF(packet != nullptr, "Not enough memory for render command list");
		commands.push_back(packet);
		return packet;
	}

}
