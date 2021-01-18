#pragma once

#include "badger/types/int_types.h"

class CPU final {

public:
	static uint32 getTotalCoreCount();
	static uint32 getCurrentCoreIndex();

};

