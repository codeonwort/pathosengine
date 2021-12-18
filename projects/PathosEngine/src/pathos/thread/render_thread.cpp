#include "render_thread.h"
#include "pathos/util/log.h"

#include "badger/thread/cpu.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	void RenderThread::renderThreadMain(RenderThread* renderThread) {
		renderThread->threadID = CPU::getCurrentThreadId();

		while (renderThread->pendingKill == false) {
			// 1. Main thread inform the render thread to render frame N.
			// 2. Main thread do game tick for frame (N+1).
			// 3. Main thread create render proxies for frame (N+1).
			// 4. Main thread waits for render thread to finish rendering frame N.
			// 5. Flip the backbuffer and go to the next frame.

			std::unique_lock<std::mutex> cvLock(renderThread->loopMutex);
			renderThread->condVar.wait(cvLock);

			// #todo-renderthread: Move Engine->render() here
			//LOG(LogDebug, "RenderThread frame=%d", renderThread->currentFrame);
		}

		LOG(LogInfo, "[%s] Render thread terminated", __FUNCTION__);
	}

	//////////////////////////////////////////////////////////

	RenderThread::RenderThread()
		: nativeThread()
		, threadID(0xffffffff)
		, threadName("render_thread")
		, currentFrame(0)
	{
		// #todo-renderthread: Set thread id and name (STL? Win32?)
	}

	RenderThread::~RenderThread() {
	}

	void RenderThread::run() {
		nativeThread = std::thread(renderThreadMain, this);
	}

	void RenderThread::beginFrame(uint32 frameNumber) {
		currentFrame = frameNumber;
		condVar.notify_all();
	}

	void RenderThread::waitForCompletion()
	{
		//
	}

	void RenderThread::endFrame(uint32 frameNumber) {
		CHECKF(currentFrame == frameNumber, "Frame number does not match!!!");
		waitForCompletion();
	}

	void RenderThread::terminate() {
		// #todo-renderthread: Does this ensure the lock will break?
		pendingKill = true;
		condVar.notify_all();
	}
	
}
