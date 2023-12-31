// ----------------------------------------------------------------------------
// Custom memory allocators
// 
// - StackAllocator
// - PoolAllocator
// - CircularAllocator
// ----------------------------------------------------------------------------

#pragma once

#include "badger/types/int_types.h"
#include "badger/assertion/assertion.h"
#include <stdlib.h>

// Fixed total bytes, suballocate only incrementally.
// Allocating more than max capacity is prohibited.
class StackAllocator
{
public:
	explicit StackAllocator(uint32 bytes);
	~StackAllocator();

	void* alloc(uint32 bytes);
	void clear();

	bool isClear() const;

private:
	void* memblock;
	void* current;
	uint32 totalBytes;
	uint32 usedBytes;
};

// Fixed max item count and all items have the same size.
// Can deallocate items for new allocations.
template<typename T>
class PoolAllocator
{
	struct FreeNode
	{
		T element;
		FreeNode* next;
	};
	
public:
	explicit PoolAllocator(uint32 maxElements)
	{
		memblock = ::malloc(maxElements * sizeof(FreeNode));
		void* current = memblock;
		FreeNode* prev = nullptr;
		for (auto i = 0u; i < maxElements; ++i)
		{
			FreeNode* node = reinterpret_cast<FreeNode*>(current);
			current = reinterpret_cast<uint8_t*>(current) + sizeof(FreeNode);
			new (&(node->element)) T();
			node->next = prev;
			prev = node;
		}
		freeList = prev;
	}
	~PoolAllocator()
	{
		::free(memblock);
	}

	T* alloc()
	{
		if (freeList == nullptr)
		{
			return nullptr;
		}
		T* elem = &(freeList->element);
		freeList = freeList->next;
		return elem;
	}

	void dealloc(T* element)
	{
		FreeNode* node = reinterpret_cast<FreeNode*>(element);
		node->next = freeList;
		freeList = node;
	}

private:
	void* memblock;
	FreeNode* freeList;
};

// Fixed max item count, suballocate only incrementally.
// Allocating more than max capacity will erase old items.
// 
// @param T : datatype of items
// @param bCallCtorAndDtor : If true, call constructor and destructor for each item.
template<typename T, bool bCallCtorAndDtor>
class CircularAllocator
{
public:
	explicit CircularAllocator(uint32 maxElements)
	{
		CHECK(maxElements > 0);
		memblock = ::malloc(maxElements * sizeof(T));
		maxCount = maxElements;
		head = 0;
		tail = 0;
	}
	~CircularAllocator()
	{
		if constexpr (bCallCtorAndDtor)
		{
			for (uint32 i = 0; i < numElements(); ++i)
			{
				T* elem = getElement(i);
				elem->~T();
			}
		}
		::free(memblock);
	}

	T* alloc()
	{
		uint8* baseAddr = reinterpret_cast<uint8*>(memblock);
		uint8* headAddr = (head * sizeof(T)) + baseAddr;
		T* elem = reinterpret_cast<T*>(headAddr);
		if constexpr (bCallCtorAndDtor)
		{
			if (tail == head + 1)
			{
				elem->~T();
			}
			new (elem) T();
		}
		++head;
		if (tail == head)
		{
			++tail;
		}
		if (head == maxCount)
		{
			head = 0;
			tail = 1;
		}
		return elem;
	}

	uint32 numElements() const
	{
		if (tail == head + 1)
		{
			return maxCount;
		}
		return head - tail;
	}

	T* getElement(uint32 ix) const
	{
		CHECK(ix < numElements());
		uint32 k = (head + ix) % maxCount;
		uint8* addr = (k * sizeof(T)) + reinterpret_cast<uint8*>(memblock);
		return reinterpret_cast<T*>(addr);
	}

private:
	void* memblock;
	uint32 maxCount;
	uint32 head; // next index to allocate
	uint32 tail; // Oldest item's index (invalid if head = 0)
};
