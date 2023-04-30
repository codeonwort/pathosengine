#include "sync_event.h"

#include "badger/system/platform.h"
#if PLATFORM_WINDOWS
	#include <Windows.h>
#endif

namespace pathos {

	SyncEvent::SyncEvent() {
#if PLATFORM_WINDOWS
		HANDLE windowsEvent = ::CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
		nativeEvent = windowsEvent;
#else
		#error Not implemented yet
#endif
	}

	SyncEvent::~SyncEvent() {
#if PLATFORM_WINDOWS
		CHECKF(nativeEvent == nullptr, "Sync event was not closed");
#else
		#error Not implemented yet
#endif
	}

	void SyncEvent::waitInfinite() {
#if PLATFORM_WINDOWS
		HANDLE windowsEvent = (HANDLE)nativeEvent;
		::WaitForSingleObject(windowsEvent, INFINITE);
#else
		#error Not implemented yet
#endif
	}

	void SyncEvent::wake() {
#if PLATFORM_WINDOWS
		HANDLE windowsEvent = (HANDLE)nativeEvent;
		::SetEvent(windowsEvent);
#else
		#error Not implemented yet
#endif
	}

	void SyncEvent::close() {
#if PLATFORM_WINDOWS
		CHECKF(nativeEvent != nullptr, "Sync event was already closed");
		::CloseHandle((HANDLE)nativeEvent);
		nativeEvent = nullptr;
#else
		#error Not implemented yet
#endif
	}

}
