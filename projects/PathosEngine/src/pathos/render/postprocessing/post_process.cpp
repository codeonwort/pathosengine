#include "post_process.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	PostProcess::~PostProcess() {
		// #todo-system: It fails on close window.
		// See DeferredRenderer::internal_destroyGlobalResources() for the reason :(
		CHECKF(resourcesDestroyed, "Child classes should override releaseResources() and set this true at the end.");
	}

}
