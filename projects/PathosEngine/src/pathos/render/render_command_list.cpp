#include "render_command_list.h"
#include "render_commands.h"
#include "pathos/thread/engine_thread.h"

#include "badger/assertion/assertion.h"

#define ASSERT_GL_NO_ERROR 0

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
#if ASSERT_GL_NO_ERROR
		glGetError();
#endif
		uint32 n = (uint32)commands.size();
		for (uint32 i = 0; i < n; ++i) {
			debugCurrentCommandIx = i;
			commands[i]->pfn_execute(commands[i]);
		}
#if ASSERT_GL_NO_ERROR
		assert(GL_NO_ERROR == glGetError());
#endif
	}

	void RenderCommandList::flushAllCommands()
	{
		std::lock_guard<std::mutex> lockGuard(commandListLock);

		++flushDepth;
		if (flushDepth == 1) {
			executeAllCommands();
			clearAllCommands();
		} else {
			CHECKF(0, "You must not nest flushAllCommands() calls");
		}
		--flushDepth;
	}

	void RenderCommandList::registerHook(std::function<void(void*)> hook, void* argument, uint64 argumentBytes)
	{
		std::lock_guard<std::mutex> lockGuard(commandListLock);

		// #todo-renderthread: Needs rework if multiple non-render threads use the deferred command list.
		// For now, it will be OK as only the main thread use it.

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
