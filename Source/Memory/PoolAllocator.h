#pragma once
#include <vector>
#include "AllocatorInterface.h"
#include <queue>
#include <functional>

#include <SDL/SDL.h>

class DECLSPEC PoolAllocator : AllocatorInterface
{
private:

	unsigned int m_maxMemory = 0;
	unsigned int m_memSlotSize = 0;

	void* m_origpointer = nullptr;
	void* m_memory = nullptr;

	std::vector<void*> m_memPoolList;
	std::priority_queue<unsigned int, std::vector<unsigned int>, std::greater<unsigned int>> m_memFreeSlots;

public:
	PoolAllocator();
	PoolAllocator(unsigned int _items, size_t _size);
	~PoolAllocator();

	void* Allocate();
	void Free(void*);
	void SetSize(unsigned int _items, size_t _size);
};
