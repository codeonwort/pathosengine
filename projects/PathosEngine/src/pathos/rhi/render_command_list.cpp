#include "render_command_list.h"
#include "render_commands.h"
#include "render_device.h"
#include "texture.h"
#include "pathos/util/engine_thread.h"

#include "badger/assertion/assertion.h"


#define ASSERT_GL_NO_ERROR 0

namespace pathos {

	const uint32 RenderCommandList::RENDER_COMMAND_LIST_MAX_MEMORY = 32 * 1024 * 1024; // 32 MB
	const uint32 RenderCommandList::COMMAND_PARAMETERS_MAX_MEMORY = 16 * 1024 * 1024; // 16 MB

	void RenderCommandList::setHookCommandList(RenderCommandList* secondaryCommandList) {
		CHECK(hookCommandList == nullptr && this != secondaryCommandList);
		hookCommandList = secondaryCommandList;
	}

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

		static constexpr bool bAllowAppendWhileExecuting = false;

		uint32 p = 0;
		while (true) {
			uint32 n = (uint32)commands.size();
			for (; p < n; ++p) {
				debugCurrentCommandIx = p;
				commands[p]->pfn_execute(commands[p]);
			}
			if (n == commands.size()) {
				break;
			}
			if (!bAllowAppendWhileExecuting) {
				CHECKF(false, "Render command list is expanded during excution which is forbidden");
			}
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
			performDeferredCleanup();
		} else {
			CHECKF(0, "You must not nest flushAllCommands() calls");
		}
		--flushDepth;
	}

	void RenderCommandList::registerHook(std::function<void(RenderCommandList& cmdList)> hook)
	{
		CHECK(hookCommandList != nullptr);

		std::lock_guard<std::mutex> lockGuard(commandListLock);

		// #todo-renderthread: Needs rework if multiple non-render threads use the deferred command list.
		// For now, it will be OK as only the main thread use it.

		RenderCommand_registerHook* __restrict packet = (RenderCommand_registerHook*)getNextPacket();
		memset(packet, 0, sizeof(RenderCommandPacketUnion));
		packet->pfn_execute = PFN_EXECUTE(RenderCommand_registerHook::execute);
		packet->hook = hook;
		packet->hookCommandList = hookCommandList;
	}

	void RenderCommandList::registerDeferredCleanup(void* dynamicMemory) {
		std::lock_guard<std::mutex> lockGuard(deferredCleanupLock);
		deferredMemoryCleanups.push_back(dynamicMemory);
	}

	void RenderCommandList::registerDeferredBufferCleanup(GLuint buffer) {
		std::lock_guard<std::mutex> lockGuard(deferredCleanupLock);
		deferredBufferCleanups.push_back(buffer);
	}

	void RenderCommandList::registerDeferredTextureCleanup(GLuint texture) {
		std::lock_guard<std::mutex> lockGuard(deferredCleanupLock);
		deferredTextureCleanups.push_back(texture);
	}

	void RenderCommandList::registerDeferredSamplerCleanup(GLuint sampler) {
		std::lock_guard<std::mutex> lockGuard(deferredCleanupLock);
		deferredSamplerCleanups.push_back(sampler);
	}

	void RenderCommandList::performDeferredCleanup() {
		std::lock_guard<std::mutex> lockGuard(deferredCleanupLock);
		for (void* memory : deferredMemoryCleanups) delete memory;
		deferredMemoryCleanups.clear();
		gRenderDevice->deleteBuffers((GLsizei)deferredBufferCleanups.size(), deferredBufferCleanups.data());
		deferredBufferCleanups.clear();
		gRenderDevice->deleteTextures((GLsizei)deferredTextureCleanups.size(), deferredTextureCleanups.data());
		deferredTextureCleanups.clear();
		gRenderDevice->deleteSamplers((GLsizei)deferredSamplerCleanups.size(), deferredSamplerCleanups.data());
		deferredSamplerCleanups.clear();
	}

	RenderCommandBase* RenderCommandList::getNextPacket()
	{
		//std::lock_guard<std::mutex> lockGuard(commandListLock);

		RenderCommandBase* packet = (RenderCommandBase*)commands_alloc.alloc(sizeof(RenderCommandPacketUnion));
		CHECKF(packet != nullptr, "Not enough memory for render command list");
		commands.push_back(packet);
		return packet;
	}

}
