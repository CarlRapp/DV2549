#include "PoolAllocator.h"
#include <stdlib.h>
#include <assert.h>

PoolAllocator::PoolAllocator(unsigned int _items, size_t _size)
{
	m_memFreeSlots = new std::priority_queue<unsigned int, std::vector<unsigned int>, std::greater<unsigned int>>();

	SetSize(_items, _size);
	m_Mutex = SDL_CreateMutex();
}

PoolAllocator::~PoolAllocator()
{
	free(m_origpointer);

	delete m_memFreeSlots;
}

void* PoolAllocator::Allocate()
{
	char* mem = static_cast<char*>(m_memory);

	SDL_LockMutex(m_Mutex);
#if MEMORY_DEBUG //&& !_DEBUG
	if (m_memFreeSlots->empty())
	{
		printf("out of memory\n");
		SDL_UnlockMutex(m_Mutex);
		return nullptr;
	}
//#elif _DEBUG
	assert(!m_memFreeSlots->empty());
#endif	

	unsigned int memPointer = m_memFreeSlots->top() * m_memSlotSize;
	m_memFreeSlots->pop();
	SDL_UnlockMutex(m_Mutex);

	return &mem[memPointer];
}

void PoolAllocator::Free(void* deleted)
{
	char* d = static_cast<char*>(deleted);
	char* m = static_cast<char*>(m_memory);

#if MEMORY_DEBUG// && !_DEBUG
	if (d < m)
	{
		printf("Free - out of bounds\n");
		return;
	}
//#elif _DEBUG
	assert(d >= m);
#endif

	unsigned int diff = static_cast<unsigned int>(d - m);

#if MEMORY_DEBUG //&& !_DEBUG
	if (diff >= m_maxMemory)
	{
		printf("Free - out of bounds\n");
		return;
	}
//#elif _DEBUG
	assert(diff < m_maxMemory);
#endif

	diff /= m_memSlotSize;

	SDL_LockMutex(m_Mutex);
	m_memFreeSlots->push(diff);
	SDL_UnlockMutex(m_Mutex);
}

bool PoolAllocator::IsEmpty()
{
	SDL_LockMutex(m_Mutex);
	bool result = m_memFreeSlots->size() == ((m_maxMemory  - MEMORY_ALIGNMENT) / m_memSlotSize);
	SDL_UnlockMutex(m_Mutex);
	return result;
}

bool PoolAllocator::HasFreeSlot()
{	SDL_LockMutex(m_Mutex);
	bool result = !m_memFreeSlots->empty();
	SDL_UnlockMutex(m_Mutex);
	return result;
}

inline bool PoolAllocator::IsInRange(void* _pointer)
{
	return (_pointer < m_maxPointer) && (_pointer >= m_memory);
}

void PoolAllocator::SetSize(unsigned int _items, size_t _size)
{

#ifdef MEMORY_PADDING
	m_memSlotSize = GetPaddedBlockSize(_size);
#else
	m_memSlotSize = _size;
#endif

#if MEMORY_DEBUG
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
	memset(m_origpointer, 0, m_maxMemory); //TODO: REMOVE?
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

	for (unsigned int i = 0; i < _items; i++)
	{
		m_memFreeSlots->push(i);
	}

	m_maxPointer = static_cast<char*>(m_origpointer) + m_maxMemory;
}

char* PoolAllocator::GetPrint()
{
	return static_cast<char*>(m_memory);
}