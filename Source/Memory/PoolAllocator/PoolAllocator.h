#pragma once
#include <vector>
#include <queue>
#include <functional>

#include <SDL/SDL.h>
#include "Memory/MemorySettings.h"

static size_t GetPaddedBlockSize(size_t _size) 
{
	int temp = MEMORY_ALIGNMENT - 1;

	return (_size + temp) & ~temp;
}

class DECLSPEC PoolAllocator
{
private:

	unsigned int m_maxMemory = 0;
	unsigned int m_memSlotSize = 0;

	void* m_origpointer = nullptr;
	void* m_memory = nullptr;

	std::priority_queue<unsigned int, std::vector<unsigned int>, std::greater<unsigned int>>* m_memFreeSlots;

	SDL_mutex* m_Mutex;

	

private:
	void SetSize(unsigned int _items, size_t _size);

public:
	PoolAllocator(unsigned int _items, size_t _size);
	~PoolAllocator();

	void* Allocate();
	void Free(void*);

	bool IsEmpty();

	bool HasFreeSlot();

	bool IsInRange(void*);
	
#ifdef MEMORY_DEBUG
	char* GetPrint();
	unsigned int GetMaxMemory() { return m_maxMemory - MEMORY_ALIGNMENT; };
#endif
	
};
