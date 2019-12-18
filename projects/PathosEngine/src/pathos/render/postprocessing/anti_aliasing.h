#pragma once

#include "badger/types/int_types.h"

namespace pathos {

	enum class EAntiAliasingMethod : uint8 {
		NoAA = 0,
		FXAA = 1,
		// #todo-anti-aliasing: TAA
		NumMethods = 2
	};

}
