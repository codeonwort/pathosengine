#pragma once

#include "badger/types/int_types.h"

class CPU final {

public:
	// Metric about logical processors
	static uint32 getTotalLogicalCoreCount();
	// #todo-cpu: Why this changes within a single function?
	static uint32 getCurrentLogicalCoreIndex();

	// #todo-cpu: Should I detect SMT?
	//static bool supportsSimultaneousMultithreading() const;

	static uint32 getCurrentThreadId();

};
