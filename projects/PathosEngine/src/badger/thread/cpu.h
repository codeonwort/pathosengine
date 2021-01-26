#pragma once

#include "badger/types/int_types.h"

class CPU final {

public:
	static uint32 getTotalLogicalCoreCount();

	// #note-cpu: It can change within a single function. Usually it's good to use thread id instead.
	static uint32 getCurrentLogicalCoreIndex();

	// #todo-cpu: If needed, search for GetLogicalProcessorInformation()
	//static bool supportsSimultaneousMultithreading() const;

	static uint32 getCurrentThreadId();

};
