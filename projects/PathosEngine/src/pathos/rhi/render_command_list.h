#pragma once

#include "render_commands.h"

#include "badger/system/mem_alloc.h"
#include "badger/assertion/assertion.h"

#include <vector>
#include <functional>
#include <mutex>

namespace pathos {

	class Texture;

	class RenderCommandList {

		static const uint32 RENDER_COMMAND_LIST_MAX_MEMORY;
		static const uint32 COMMAND_PARAMETERS_MAX_MEMORY;

	private:
		struct RenderCommand_registerHook : RenderCommandBase {
			std::function<void(RenderCommandList& cmdList)> hook;
			RenderCommandList* hookCommandList;

			static void APIENTRY execute(const RenderCommand_registerHook* __restrict params) {
				params->hook(*(params->hookCommandList));
				params->hookCommandList->flushAllCommands();
			}
		};
		
	public:
		RenderCommandList(const char* inDebugName, uint32 commandAllocBytes = RENDER_COMMAND_LIST_MAX_MEMORY, uint32 parametersAllocBytes = COMMAND_PARAMETERS_MAX_MEMORY)
			: debugName(inDebugName)
			, debugCurrentCommandIx(0)
			, commands_alloc(StackAllocator(commandAllocBytes))
			, parameters_alloc(StackAllocator(parametersAllocBytes))
			, sceneProxy(nullptr)
			, sceneRenderTargets(nullptr)
			, hookCommandList(nullptr)
		{
		}

		void setHookCommandList(RenderCommandList* secondaryCommandList);

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
		void registerHook(std::function<void(RenderCommandList& cmdList)> hook);

		// Returns a memory region that will be invalidated after all commands are executed.
		void* allocateSingleFrameMemory(uint64 bytes) {
			CHECK(bytes <= 0xffffffff); // Well, who would do this?
			void* mem = parameters_alloc.alloc((uint32)bytes);
			CHECK(mem != nullptr);
			return mem;
		}

		// Frees the memory after all commands are executed.
		// CAUTION: The parameter should point a dynamically allocated memory and not free'd in other places.
		void registerDeferredCleanup(void* dynamicMemory);
		void registerDeferredBufferCleanup(GLuint buffer);
		void registerDeferredTextureCleanup(GLuint texture);

		// Free all memory that came from registerDeferredCleanup().
		void performDeferredCleanup();

		inline uint32 getNumCommands() const { return (uint32)commands.size(); }
		inline bool isEmpty() const { return commands.size() == 0; }

		// Debug only
		const char* debugName;
		uint32 debugCurrentCommandIx;

		// Should be assigned by renderer before rendering anything of current frame
		class SceneProxy* sceneProxy;
		struct SceneRenderTargets* sceneRenderTargets;

	private:
		RenderCommandBase* getNextPacket();

		// #todo-refactoring: Use allocateSingleFrameMemory() instead.
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
		
		std::mutex commandListLock;
		std::vector<RenderCommandBase*> commands;
		uint32 flushDepth = 0;

		RenderCommandList* hookCommandList;

		std::mutex deferredCleanupLock;
		std::vector<void*> deferredMemoryCleanups;
		std::vector<GLuint> deferredBufferCleanups;
		std::vector<GLuint> deferredTextureCleanups;

	public:
		#include "render_command_list.generated.h"

	};

}
