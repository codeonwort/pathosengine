#pragma once

#include "badger/types/int_types.h"
#include "badger/system/platform.h"

#if PLATFORM_WINDOWS
	using PlatformThreadId = uint32;
#else
	#error "PlatformThreadId is undefined for the target platform."
#endif

class CPU final {

public:
	static uint32 getTotalLogicalCoreCount();

	// #note-cpu: It can change within a single function. Usually it's good to use thread id instead.
	static uint32 getCurrentLogicalCoreIndex();

	static PlatformThreadId getCurrentThreadId();

	static void setCurrentThreadName(const wchar_t* name);

};
