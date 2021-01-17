#include "mem_alloc.h"
#include "badger/assertion/assertion.h"

StackAllocator::StackAllocator(uint32 bytes)
{
	memblock = malloc(bytes);
	current = memblock;
	totalBytes = bytes;
	usedBytes = 0;
}

StackAllocator::~StackAllocator()
{
	free(memblock);
}

void* StackAllocator::alloc(uint32 bytes)
{
	CHECK(bytes > 0);

	if (usedBytes + bytes > totalBytes)
	{
		return nullptr;
	}

	void* block = (void*)(reinterpret_cast<uint8_t*>(memblock) + usedBytes);
	usedBytes += bytes;

	return block;
}

void StackAllocator::clear()
{
	usedBytes = 0;
}

bool StackAllocator::isClear() const
{
	return usedBytes == 0;
}
