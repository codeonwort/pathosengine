#pragma once

#include "render_commands.h"
#include "badger/memory/mem_alloc.h"
#include <vector>

namespace pathos {

	static constexpr uint32 RENDER_COMMAND_LIST_MAX_MEMORY = 16 * 1024 * 1024; // 16 MB

	class RenderCommandList {
		
	public:
		RenderCommandList()
			: commands_alloc(StackAllocator(RENDER_COMMAND_LIST_MAX_MEMORY))
		{
		}

		void reset();

	private:
		RenderCommandBase* getNextPacket();

		StackAllocator commands_alloc;
		std::vector<RenderCommandBase*> commands;

	public:
		#include "render_command_list.generated.h"

	};

}