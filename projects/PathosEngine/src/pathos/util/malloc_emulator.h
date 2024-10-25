#pragma once

#include "badger/types/int_types.h"
#include <type_traits>

namespace pathos {

	/// Used to emulate dynamic allocation of GPU resource from C++ side.
	class MallocEmulator {

		struct Range {
			uint64 offset;
			uint64 bytes;
			Range* left; // if left == this, it means the range is allocated.
			Range* right;
			Range* parent;

			inline void markResident() { left = this; }
			inline void evict() { left = nullptr; }
			inline bool isResident() const { return left == this; }
			inline bool hasChild() const { return right != nullptr; } // Always create two children, so only check the right one.
		};

	public:
		static constexpr uint64 INVALID_OFFSET = static_cast<uint64>(-1);

		~MallocEmulator();

		void initialize(uint64 totalBytes, uint64 alignment = 0);

		void cleanup();

		/// Returns offset.
		uint64 allocate(uint64 bytes);

		/// The argument must be an offset previously returned by allocate().
		void deallocate(uint64 offset);

		/// NOTE: It's not gauranteed that allocate(remainingBytes()) succeed due to internal fragmentation.
		inline uint64 getRemainingBytes() const { return remainingBytes; }

		/// The number of active allocations.
		inline uint64 getNumAllocations() const { return numAllocations; }

	private:
		Range* merge(Range* node);
		void debugTopology();

		Range* root = nullptr;
		uint64 remainingBytes = 0;
		uint64 numAllocations = 0;
		uint64 alignment = 0;

	};

}
