#pragma once

#include "render_commands.h"

#include "badger/memory/mem_alloc.h"
#include "badger/assertion/assertion.h"

#include <vector>
#include <functional>

namespace pathos {

	class RenderCommandList {

		static const uint32 RENDER_COMMAND_LIST_MAX_MEMORY;
		static const uint32 COMMAND_PARAMETERS_MAX_MEMORY;

	private:
		struct RenderCommand_registerHook : RenderCommandBase {
			std::function<void(void*)> hook;
			void* argument;

			static void APIENTRY execute(const RenderCommand_registerHook* __restrict params) {
				params->hook(params->argument);
			}
		};
		
	public:
		RenderCommandList(uint32 commandAllocBytes = RENDER_COMMAND_LIST_MAX_MEMORY, uint32 parametersAllocBytes = COMMAND_PARAMETERS_MAX_MEMORY)
			: commands_alloc(StackAllocator(commandAllocBytes))
			, parameters_alloc(StackAllocator(parametersAllocBytes))
			, sceneRenderTargets(nullptr)
		{
		}

		// Clear all commands in the list.
		void clearAllCommands();

		// Execute all commands in the list. Does not clear the list.
		void executeAllCommands();

		/**
		 * Simply call executeAllCommands() and then clearAllCommands().
		 * CAUTION: Do not confuse this with flush() which is glFlush().
		 */ 
		void flushAllCommands();

		// For subsequent works related to GL calls that have return values
		void registerHook(std::function<void(void*)> hook, void* argument, uint64 argumentBytes);

		// Should be assigned by renderer before rendering anything of current frame
		struct SceneRenderTargets* sceneRenderTargets;

		uint32 debugCurrentCommandIx = 0;

	private:
		RenderCommandBase* getNextPacket();

		template<typename T>
		T* storeParameter(uint64 bytes, T* data)
		{
			CHECK(bytes <= 0xffffffff); // Well, who would do this?
			T* mem = (T*)parameters_alloc.alloc((uint32)bytes);
			CHECK(mem != nullptr);
			memcpy_s((void*)mem, bytes, (void*)data, bytes);
			return mem;
		}

		StackAllocator commands_alloc;
		StackAllocator parameters_alloc; // non-singular parameters should be mem-copied to this allocator, as source data could be a local variable

		std::vector<RenderCommandBase*> commands;
		uint32 flushDepth = 0;

	public:
		#include "render_command_list.generated.h"

	};

}
