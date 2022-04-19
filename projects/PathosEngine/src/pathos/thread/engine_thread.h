#pragma once

#include "badger/types/int_types.h"
#include "badger/thread/cpu.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	extern uint32 gMainThreadId;
	extern uint32 gRenderThreadId;

	inline bool isInMainThread() {
		CHECK(gMainThreadId != 0);
		return (gMainThreadId == CPU::getCurrentThreadId());
	}

	inline bool isInRenderThread() {
		CHECK(gRenderThreadId != 0);
		return (gRenderThreadId == CPU::getCurrentThreadId());
	}

}
