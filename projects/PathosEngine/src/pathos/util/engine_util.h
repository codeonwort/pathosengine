#pragma once

#include "pathos/engine.h"
#include "pathos/render/render_command_list.h"
using namespace pathos;

namespace pathos {

	class EngineUtil {
		
	public:
		static StackAllocator& getRenderProxyAllocator() {
			return gEngine->renderProxyAllocator;
		}

	};

	// #note-fbo: For renderstate validation. No need for release build.
	void checkFramebufferStatus(RenderCommandList& cmdList, GLuint fbo, const char* message = nullptr);

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

inline const char* ALLOC_PDO_STRING(const char* str) {
	CHECKF(gEngine != nullptr, "Engine instance is invalid");

	if (str == nullptr) {
		return nullptr;
	}

	size_t len = strlen(str);
	char* proxy = reinterpret_cast<char*>(EngineUtil::getRenderProxyAllocator().alloc((uint32)(len + 1)));
	strcpy_s(proxy, len + 1, str);
	return proxy;
}

// Use inside of pathos namespace
#define DEFINE_GLOBAL_RENDER_ROUTINE(UniqueName, InitRoutine, DestroyRoutine)           \
	static struct _GlobalRenderRoutine_##UniqueName {	   							    \
		_GlobalRenderRoutine_##UniqueName() {											\
			Engine::internal_registerGlobalRenderRoutine(InitRoutine, DestroyRoutine);	\
		}																				\
	} _global_render_routine_##UniqueName;                                          	\
