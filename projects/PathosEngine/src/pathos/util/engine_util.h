#pragma once

#include "pathos/engine.h"
#include "pathos/rhi/render_command_list.h"
#include "pathos/render/scene_proxy.h"
using namespace pathos;

namespace pathos {

	// #note-fbo: For renderstate validation. No need for release build.
	void checkFramebufferStatus(RenderCommandList& cmdList, GLuint fbo, const char* message = nullptr);

}

// #todo-render-proxy: For performance, CHECK() and placement new might be disabled later.
template<typename T>
inline T* ALLOC_RENDER_PROXY(SceneProxy* scene) {
	StackAllocator& allocator = scene->renderProxyAllocator;

	T* proxy = reinterpret_cast<T*>(allocator.alloc(sizeof(T)));
#if defined(_DEBUG)
	CHECKF(proxy != nullptr, "Failed to allocate render proxy!!! Need to increase the allocator size.");
#endif
	new (proxy) T;
	return proxy;
}

inline const char* ALLOC_PDO_STRING(StackAllocator& allocator, const char* str) {
	if (str == nullptr) {
		return nullptr;
	}

	size_t len = strlen(str);
	char* proxy = reinterpret_cast<char*>(allocator.alloc((uint32)(len + 1)));
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
