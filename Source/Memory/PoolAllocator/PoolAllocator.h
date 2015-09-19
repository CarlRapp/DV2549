#pragma once
#include <vector>
#include <queue>
#include <functional>

#include <SDL/SDL.h>
#include "Memory/MemorySettings.h"

inline static size_t GetPaddedBlockSize(size_t _size) 
{
	int temp = MEMORY_ALIGNMENT - 1;

	return (_size + temp) & ~temp;
}

struct myQueue
{
	/*
	
	Increase free slots --->

	Memory:
	[L][x][x][x][x][x][x][x][R][][][][][][][Size]
	
	x = free slot
	R++ on push
	L++ on top
	L > Size : L = 0
	R > Size : R = 0

	*/

	unsigned int* Slots;
	unsigned int Right = 0;
	unsigned int Left = 0;
	unsigned int Size = 0;

	void alloc(unsigned int _size)
	{
		Slots = static_cast<unsigned int*>(malloc(_size*sizeof(unsigned int)));
		Size = _size;
	}

	void push(unsigned int _in)
	{
		Right++;
		if (Right > Size)
			Right = 1;
		Slots[Right -1] = _in;
	}

	unsigned int top()
	{
		Left++;
		if (Left > Size)
			Left = 1;
		return Slots[Left-1];
	}

	unsigned int freeSlots()
	{
		return Right > Left ? Right - Left : Size - Left + Right;
	}


};

class DECLSPEC PoolAllocator
{
private:

	unsigned int m_maxMemory = 0;
	unsigned int m_memSlotSize = 0;

	void* m_origpointer = nullptr;
	void* m_memory = nullptr;
	void* m_maxPointer = nullptr;

	void** m_first = nullptr;
	void** m_last = nullptr;

	//std::queue<unsigned int>* m_memFreeSlots;

	SDL_mutex* m_Mutex;

	myQueue m_memFreeSlots;

private:
	void SetSize(unsigned int _items, size_t _size);

public:
	PoolAllocator(unsigned int _items, size_t _size);
	~PoolAllocator();

	void* Allocate();
	void Free(void*);

	bool IsEmpty();

	bool HasFreeSlot();

	inline bool IsInRange(void*);
	
#ifdef MEMORY_DEBUG
	char* GetPrint();
	unsigned int GetMaxMemory() { return m_maxMemory - MEMORY_ALIGNMENT; };
	size_t GetFreeSlots() { return m_memFreeSlots.Right - m_memFreeSlots.Left; }
	unsigned int GetMaxSlots() { return ((m_maxMemory - MEMORY_ALIGNMENT) / m_memSlotSize); }
#endif
	
};
