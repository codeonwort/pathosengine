#pragma once

namespace pathos {

	// #todo-renderthread: PATHOS engine loop is single-threaded yet
	inline bool isInMainThread() {
		return true;
	}

	// #todo-renderthread: PATHOS engine loop is single-threaded yet
	inline bool isInRenderThread() {
		return true;
	}

}
