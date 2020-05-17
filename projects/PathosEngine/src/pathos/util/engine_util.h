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

template<typename T>
inline T* ALLOC_RENDER_PROXY() {
	CHECKF(gEngine != nullptr, "Engine instance is invalid");

	T* proxy = reinterpret_cast<T*>(EngineUtil::getRenderProxyAllocator().alloc(sizeof(T)));
	new (proxy) T; // #todo-render-proxy: Needed?
	return proxy;
}
