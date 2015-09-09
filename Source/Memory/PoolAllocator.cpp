#include "PoolAllocator.h"
#include <stdlib.h>

#define MemoryAligned

PoolAllocator::PoolAllocator()
{

}

PoolAllocator::PoolAllocator(unsigned int _items, size_t _size)
{
	SetSize(_items, _size);
}

PoolAllocator::~PoolAllocator()
{
	free(m_origpointer);
}

void* PoolAllocator::Allocate()
{
	char* mem = static_cast<char*>(m_memory);


	if (m_memFreeSlots.empty())
	{
		printf("out of memory\n");
		return nullptr;
	}

	unsigned int memPointer = m_memFreeSlots.top() * m_memSlotSize;
	m_memFreeSlots.pop();

	return &mem[memPointer];
}

void PoolAllocator::Free(void* deleted)
{
	char* d = static_cast<char*>(deleted);
	char* m = static_cast<char*>(m_memory);

	int diff = static_cast<int>(d - m);

	diff /= m_memSlotSize;

	m_memFreeSlots.push(diff);
}

void PoolAllocator::SetSize(unsigned int _items, size_t _size)
{

#if defined(_WIN64)
	int alignment = 8;
	//#elseif defined(__x86__)
#elif defined(_WIN32)
	int alignment = 4;
#endif

	m_maxMemory = _items;
	m_origpointer = malloc(_size * _items + alignment);
	m_memory = m_origpointer;

	size_t align = (size_t)(m_memory);
		
	int unaligned = align & (alignment * 2 - 1);

	if (unaligned == 0)
	{
		printf("Memory aligned\n");
	}

	else
	{
#ifdef MemoryAligned
		m_memory = (char*)m_origpointer + alignment - unaligned;
#endif
		printf("Memory not aligned\n");
	}

	m_memSlotSize = _size;


	for (unsigned int i = 0; i < _items; i++)
	{
		m_memFreeSlots.push(i);
	}
}

