#pragma once

namespace pathos {

	// #todo-multi-thread: PATHOS engine loop is single-threaded yet
	inline bool isInMainThread() {
		return true;
	}

	// #todo-multi-thread: PATHOS engine loop is single-threaded yet
	inline bool isInRenderThread() {
		return true;
	}

}
