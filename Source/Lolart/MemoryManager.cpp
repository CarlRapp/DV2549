#include "MemoryManager.h"
#include <stdlib.h>

void* MemoryManager::Allocate(size_t size)
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

void MemoryManager::Free(void* deleted)
{
	int* d = static_cast<int*>(deleted);
	int* m = static_cast<int*>(m_memory);

	int diff = &d - &m;

	diff /= m_memSlotSize;

	m_memSlotsTaken[diff] = false;

// 	FreeStore* head = static_cast <FreeStore*> (deleted);
// 	head->next = freeStoreHead;
// 	freeStoreHead = head;
}

void MemoryManager::SetSize(unsigned int _items, size_t _size)
{
	m_maxMemory = _items;
	m_memory = malloc(_size*_items);

	m_memSlotSize = _size;

	m_memSlotsTaken.resize(_items);
	for (int i = 0; i < m_memSlotsTaken.size(); i++)
	{
		m_memSlotsTaken[i] = false;
	}
}

MemoryManager::MemoryManager()
{

}

MemoryManager::~MemoryManager()
{

}
