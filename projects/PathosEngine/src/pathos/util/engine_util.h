#pragma once

#include "pathos/engine.h"
using namespace pathos;

namespace pathos {

	class EngineUtil {
		
	public:
		static StackAllocator& getRenderProxyAllocator() {
			return gEngine->renderProxyAllocator;
		}

	};

}

// #todo-render-proxy: For performance, CHECK() and placement new might be disabled later.
template<typename T>
inline T* ALLOC_RENDER_PROXY() {
	CHECKF(gEngine != nullptr, "Engine instance is invalid");

	T* proxy = reinterpret_cast<T*>(EngineUtil::getRenderProxyAllocator().alloc(sizeof(T)));
#if defined(_DEBUG)
	CHECKF(proxy != nullptr, "Failed to allocate render proxy!!! Need to increase the allocator size.");
#endif
	new (proxy) T;
	return proxy;
}
