#include "PoolAllocator.h"
#include <stdlib.h>
#include <assert.h>

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
#if MEMORY_DEBUG 
	assert(m_first);
	assert(m_freeslots != 0);
#endif
	void* result = m_first;
	m_first = static_cast<void**>(*m_first);
	--m_freeslots;
	return result;
}

void PoolAllocator::Free(void* deleted)
{
#if MEMORY_DEBUG 
	char* d = static_cast<char*>(deleted);
	char* m = static_cast<char*>(m_memory);
	unsigned int diff = static_cast<unsigned int>(d - m);
	assert(d >= m && diff >= m_maxMemory);
	assert(m_last);
#endif

	*m_last = deleted;
	m_last = static_cast<void**>(deleted);
	*m_last = nullptr;
	++m_freeslots;
}


bool PoolAllocator::HasFreeSlot()
{	
	return m_freeslots != 0;
}

inline bool PoolAllocator::IsInRange(void* _pointer)
{
	return (_pointer < m_maxPointer) && (_pointer >= m_memory);
}

void PoolAllocator::SetSize(unsigned int _items, size_t _size)
{

#ifdef MEMORY_PADDING
	m_memSlotSize = (unsigned int)GetPaddedBlockSize(_size);
#else
	m_memSlotSize = _size;
#endif

#if MEMORY_DEBUG

#if !MEMORY_ALIGNED
	assert(_size >= MEMORY_ALIGNMENT);
#endif

	if (m_memSlotSize == _size)
	{
		printf("Blocksize was already aligned\n");
	}
	else
	{
		printf("Blocksize needed padding\n");
	}
#endif


	m_maxMemory = m_memSlotSize * _items + MEMORY_ALIGNMENT;
	m_origpointer = malloc(m_maxMemory);
	m_memory = m_origpointer;

	size_t align = (size_t)(m_memory);
		
	int unaligned = align & (MEMORY_ALIGNMENT * 2 - 1);

	if (unaligned != 0)
	{
#ifdef MEMORY_ALIGNED
		m_memory = (char*)m_origpointer + MEMORY_ALIGNMENT - unaligned;
#endif
#if MEMORY_DEBUG
		printf("OS Pointer was NOT aligned %p\n", m_origpointer);
#endif
	}
#if MEMORY_DEBUG
	else
	{
		printf("OS Pointer OK %p\n", m_origpointer);
	}
#endif

	char* start = static_cast<char*>(m_memory);
	for (unsigned int i = 0; i < _items - 1; i++)
	{
		char* a = (start + m_memSlotSize*(i + 1));
		memcpy(start + m_memSlotSize*i, &a, sizeof(char*));
	}
	memset(start + m_memSlotSize*(_items - 1), 0, sizeof(char*));

	m_first = reinterpret_cast<void**>(start);
	m_last = reinterpret_cast<void**>(start + m_memSlotSize*(_items - 1));

	m_freeslots = _items;

	m_maxPointer = static_cast<char*>(m_origpointer) + m_maxMemory;
}

char* PoolAllocator::GetPrint()
{
	return static_cast<char*>(m_memory);
}