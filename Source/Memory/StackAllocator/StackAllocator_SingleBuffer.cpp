#include "StackAllocator_SingleBuffer.h"
#include <iostream>
#include <assert.h>

using namespace Memory;

StackAllocator_SingleBuffer::StackAllocator_SingleBuffer(size_t _stackSizeBytes, size_t _stackAlignment, bool _growIfFull, bool _allocateIfFull)
	: m_stackStartPtr(0), m_alignBytes(_stackAlignment)
{
	//	Check so alignment is greater than one
	assert(m_alignBytes > 1);

	//	Allocate the number of requested bytes plus requested alignment minus on
	//	to make sure that the amount of requested bytes can be used. 
	//	(Worst case scenario, m_alignBytes-1 needs to be used to 'move' the pointer)
	m_stackStartPtr = (char*)malloc(_stackSizeBytes + (m_alignBytes - 1));

	//	Check so we got a valid pointer
	assert(m_stackStartPtr != 0);

	//	Alignment must be a power of two
	assert(!(m_alignBytes & (m_alignBytes-1)));
	

	//	Calculate how much to align the start of the pointer
	size_t bytesToAlign = ((size_t)m_stackStartPtr) % m_alignBytes;
	m_startPtrOffset = bytesToAlign;
	FreeTo(0);

	m_totalByteSize = _stackSizeBytes;
	m_growIfFull = _growIfFull;
	m_allocateIfFull = _allocateIfFull;
	m_reserveMutex = SDL_CreateMutex();

	//	Create bitmask
	m_alignBitmask = ~(m_alignBytes);
}

StackAllocator_SingleBuffer::~StackAllocator_SingleBuffer()
{
	free(m_stackStartPtr);
}

bool Memory::StackAllocator_SingleBuffer::HasPointer(void * _inPtr)
{
	size_t ptrValue = (size_t)_inPtr;
	size_t stackBlockPtr = (size_t)m_stackStartPtr + m_startPtrOffset;

	if (stackBlockPtr <= ptrValue && ptrValue < stackBlockPtr + m_totalByteSize)
		return true;

	return false;
}

//This method will return a NULL pointer if the stack can' allocated the requested amount of bytes.
void* StackAllocator_SingleBuffer::Reserve(size_t _nBytes)
{
	void* returnPtr = 0;

	//	Calculate padding
	size_t bytesToReserve = (_nBytes + m_alignBytes-1) & ~(m_alignBytes-1);

	//	Fetch the current stack index and then increment it
	//	with the requested bytes together with additional padding
	size_t preIncrementIndex = m_currentStackIndex.fetch_add(bytesToReserve, std::memory_order_relaxed);

	//	Check so the index is within memory bounds
	if (preIncrementIndex + bytesToReserve <= m_totalByteSize)
		returnPtr = &m_stackStartPtr[preIncrementIndex];

	return returnPtr;
}

size_t StackAllocator_SingleBuffer::GetTop()
{
	return m_currentStackIndex;
}

void StackAllocator_SingleBuffer::FreeTo(size_t _index)
{
	m_currentStackIndex = m_startPtrOffset + _index;
}