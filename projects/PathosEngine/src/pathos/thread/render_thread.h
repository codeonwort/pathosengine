#pragma once

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"

#include <string>
#include <thread>
#include <condition_variable>

namespace pathos {

	// #todo-renderthread: Designed only render thread in mind, but might be re-implemented with generalized Thread.
	class RenderThread final : public Noncopyable {
		static void renderThreadMain(RenderThread* renderThread);

	public:
		RenderThread();
		~RenderThread();

		void run();

		void beginFrame(uint32 frameNumber);

		void endFrame();

		void terminate();

		inline uint32 getThreadID() const { return threadID; }
		inline const std::string& getThreadName() const { return threadName; }

	private:
		uint32                     threadID;
		std::string                threadName;
		std::thread                nativeThread;
		std::condition_variable    condVar;
		std::atomic<bool>          pendingKill;

		uint32                     currentFrame;

	};

}
