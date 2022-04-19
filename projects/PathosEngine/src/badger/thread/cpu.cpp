#include "cpu.h"
#include "badger/system/platform.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

uint32 CPU::getTotalLogicalCoreCount() {
#if PLATFORM_WINDOWS
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (uint32)info.dwNumberOfProcessors;
#else
	#error "Not implemented"
#endif
}

uint32 CPU::getCurrentLogicalCoreIndex() {
#if PLATFORM_WINDOWS
	PROCESSOR_NUMBER info;
	GetCurrentProcessorNumberEx(&info);
	uint32 index = (uint32)info.Number + ((uint32)info.Group * 64);
	return index;
#else
	#error "Not implemented"
#endif
}

uint32 CPU::getCurrentThreadId()
{
#if PLATFORM_WINDOWS
	static_assert(sizeof(DWORD) == sizeof(uint32), "Should match");
	return (uint32)GetCurrentThreadId();
#else
	#error "Not implemented"
#endif
}

void CPU::setCurrentThreadName(const wchar_t* name) {
#if PLATFORM_WINDOWS
	::SetThreadDescription(GetCurrentThread(), name);
#else
	#error "Not implemented"
#endif
}
