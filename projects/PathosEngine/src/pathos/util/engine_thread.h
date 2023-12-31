#pragma once

#include "badger/types/int_types.h"
#include "badger/assertion/assertion.h"
#include "badger/system/cpu.h"

namespace pathos {

	extern PlatformThreadId gMainThreadId;
	extern PlatformThreadId gRenderThreadId;

	inline bool isInMainThread() {
		CHECK(gMainThreadId != 0);
		return (gMainThreadId == CPU::getCurrentThreadId());
	}

	inline bool isInRenderThread() {
		CHECK(gRenderThreadId != 0);
		return (gRenderThreadId == CPU::getCurrentThreadId());
	}

}
