#pragma once

#include "badger/system/platform.h"
#include "badger/assertion/assertion.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace pathos {

#if PLATFORM_WINDOWS
	class OpenGLContextManager_Windows {

	public:
		// Call this right after the GL context is created.
		static void initialize();

		// Call this in a thread that needs the GL context.
		static void takeContext();
		// Call this in the thread that is done with the context.
		static void returnContext();

		static bool isContextTaken();

		static HDC hdc;
		static HGLRC glContext;
		static DWORD contextOwnerThreadId;

	};
#endif

}

#if PLATFORM_WINDOWS
	#define OpenGLContextManager OpenGLContextManager_Windows
#else
	#error "Needs a platform-specific implementation of OpenGLContextManager."
#endif

namespace pathos {
	struct ScopedOpenGLContext {
		ScopedOpenGLContext() { OpenGLContextManager::takeContext(); }
		~ScopedOpenGLContext() { OpenGLContextManager::returnContext(); }
	};
}
#define SCOPED_TAKE_GL_CONTEXT() pathos::ScopedOpenGLContext __scoped_gl_context;
#define CHECK_GL_CONTEXT_TAKEN() CHECK(pathos::OpenGLContextManager::isContextTaken());