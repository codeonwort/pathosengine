// Custom memory allocators.

#pragma once

#include "badger/types/int_types.h"
#include <stdlib.h>

class StackAllocator
{

public:
	explicit StackAllocator(uint32 bytes);
	~StackAllocator();

	void* alloc(uint32 bytes);
	void clear();

private:
	void* memblock;
	void* current;
	uint32 totalBytes;
	uint32 usedBytes;

};

template<typename T>
class PoolAllocator
{
	struct FreeNode
	{
		T element;
		FreeNode* next;
	};
	
public:
	explicit PoolAllocator(uint32 numElements)
	{
		memblock = malloc(numElements * sizeof(FreeNode));
		void* current = memblock;
		FreeNode* prev = nullptr;
		for (auto i = 0u; i < numElements; ++i)
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
		free(memblock);
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
