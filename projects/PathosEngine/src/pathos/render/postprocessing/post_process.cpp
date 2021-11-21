#include "post_process.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	PostProcess::~PostProcess() {
		CHECKF(resourcesDestroyed, "Child classes should override releaseResources() and set this true at the end.");
	}

}
