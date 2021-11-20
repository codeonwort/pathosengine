#include "render_thread.h"

#include "badger/thread/cpu.h"

namespace pathos {

	void RenderThread::renderThreadMain(RenderThread* renderThread) {
		renderThread->threadID = CPU::getCurrentThreadId();

		while (renderThread->pendingKill == false) {
			// #todo-renderthread: Is it possible with freeglut?
			// 1. Main thread inform the render thread to render frame N.
			// 2. Main thread do game tick for frame (N+1).
			// 3. Main thread create render proxies for frame (N+1).
			// 4. Main thread waits for render thread to finish rendering frame N.
			// 5. Flip the backbuffer and go to the next frame.
			break;
		}
	}

	//////////////////////////////////////////////////////////

	RenderThread::RenderThread()
		: nativeThread()
		, threadID(0xffffffff)
		, threadName("render_thread")
		, currentFrame(0)
	{}

	RenderThread::~RenderThread() {
	}

	void RenderThread::run() {
		nativeThread = std::thread(renderThreadMain, this);
	}

	void RenderThread::beginFrame(uint32 frameNumber) {
		currentFrame = frameNumber;
	}

	void RenderThread::endFrame() {
		//
	}

	void RenderThread::terminate() {
		pendingKill = true;
	}
	
}
