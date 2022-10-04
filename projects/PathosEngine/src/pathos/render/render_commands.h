#pragma once

#include "badger/types/int_types.h"
#include <GL/gl3w.h>

namespace pathos {

	struct RenderCommandBase;
	union RenderCommandPacketUnion;

	typedef void (APIENTRYP PFN_EXECUTE)(const RenderCommandBase* __restrict params);

	struct RenderCommandBase {
		PFN_EXECUTE pfn_execute;
	};

	#include "render_commands.generated.h"

}
