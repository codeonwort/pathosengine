#include "gl_context_manager.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

#if PLATFORM_WINDOWS
namespace pathos {

	HDC OpenGLContextManager_Windows::hdc = NULL;
	HGLRC OpenGLContextManager_Windows::glContext = NULL;
	DWORD OpenGLContextManager_Windows::contextOwnerThreadId = 0;

	void OpenGLContextManager_Windows::initialize() {
		static bool bFirst = true;
		CHECK(bFirst);
		if (bFirst) {
			bFirst = false;

			hdc = wglGetCurrentDC();
			glContext = wglGetCurrentContext();
			CHECK(hdc != NULL);
			CHECK(glContext != NULL);

			contextOwnerThreadId = GetCurrentThreadId();
		}
	}

	void OpenGLContextManager_Windows::takeContext() {
		CHECKF(contextOwnerThreadId == 0, "GL context is already taken");

		// #todo-fatal: Am I doing this wrong or wglMakeCurrent() actually can fail?
		// When failed, GetLastError() returns 2004 or 6.
		BOOL result = wglMakeCurrent(hdc, glContext);
#if 0
		if (result == false) {
			DWORD lastError = ::GetLastError();
			LOG(LogFatal, "wglMakeCurrent() failed, GetLastError()=%u", lastError);
			result = wglMakeCurrent(hdc, glContext);
		}
#else
		int32 failCount = 0;
		while (result == false && failCount++ < 10) {
			DWORD lastError = ::GetLastError();
			LOG(LogError, "wglMakeCurrent() failed, GetLastError()=%u", lastError);
			result = wglMakeCurrent(hdc, glContext);
		}
#endif
		CHECKF(result, "Failed to switch GL context");

		contextOwnerThreadId = GetCurrentThreadId();
	}

	void OpenGLContextManager_Windows::returnContext() {
		CHECK(contextOwnerThreadId == GetCurrentThreadId());

		// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-wglmakecurrent
		// MSDN says if hglrc is NULL then hdc is ignored, but actually it does not?
		BOOL result = wglMakeCurrent(/*hdc*/NULL, NULL);
		if (result == false) {
			DWORD lastError = ::GetLastError();
			LOG(LogFatal, "wglMakeCurrent() failed, GetLastError()=%u", lastError);
		}
		contextOwnerThreadId = 0;
	}

	bool OpenGLContextManager_Windows::isContextTaken() {
		return contextOwnerThreadId != 0 && contextOwnerThreadId == GetCurrentThreadId();
	}

}
#endif
