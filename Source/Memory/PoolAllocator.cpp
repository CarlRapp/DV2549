#include "PoolAllocator.h"
#include <stdlib.h>

PoolAllocator::PoolAllocator()
{

}

PoolAllocator::~PoolAllocator()
{

}

void* PoolAllocator::Allocate(size_t size)
{
	char* mem = static_cast<char*>(m_memory);

	bool outOfMemory = false;

	while (m_memSlotsTaken[m_memPointer] == 1)
	{
		m_memPointer += 1;

		if (m_memPointer >= m_maxMemory)
		{
			if (outOfMemory)
			{
				printf("out of memory\n");
				return nullptr;
			}

			m_memPointer = 0;
			outOfMemory = true;
		}
	}
	
	m_memSlotsTaken[m_memPointer] = true;
	return &mem[m_memPointer*m_memSlotSize];
}

void PoolAllocator::Free(void* deleted)
{
	int* d = static_cast<int*>(deleted);
	int* m = static_cast<int*>(m_memory);

	int diff = &d - &m;

	diff /= m_memSlotSize;

	m_memSlotsTaken[diff] = false;
}

void PoolAllocator::SetSize(unsigned int _items, size_t _size)
{
	m_maxMemory = _items;
	m_memory = malloc(_size*_items);

	size_t align = (size_t)(m_memory);
		
	if (align > 1)
	{
		--align;
		align |= align >> 1;
		align |= align >> 2;
		align |= align >> 4;
		align |= align >> 8;
		align |= align >> 16;
#ifdef __x64__
		align |= align >> 32;
#endif

		align++;
	}

	if (align != 16)
		printf("unaligned");


	m_memSlotSize = _size;

	m_memSlotsTaken.resize(_items);
	for (int i = 0; i < m_memSlotsTaken.size(); i++)
	{
		m_memSlotsTaken[i] = false;
	}
}

