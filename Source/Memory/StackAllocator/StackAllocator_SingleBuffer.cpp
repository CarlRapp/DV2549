#include "StackAllocator_SingleBuffer.h"
#include <iostream>
#include <assert.h>

using namespace Memory;

StackAllocator_SingleBuffer::StackAllocator_SingleBuffer(size_t _stackSizeBytes, size_t _stackAlignment, bool _growIfFull, bool _allocateIfFull)
	: m_stackStartPtr(0), m_alignBytes(_stackAlignment)
{
	//	Allocate the number of requested bytes plus requested alignment minus on
	//	to make sure that the amount of requested bytes can be used. 
	//	(Worst case scenario, m_alignBytes-1 needs to be used to 'move' the pointer)
	m_stackStartPtr = (char*)malloc(_stackSizeBytes + (m_alignBytes-1));
	if (m_stackStartPtr == 0)
		throw std::string("Function malloc() failed to allocate valid memory!");

	//	Calculate how much to align the start of the pointer
	size_t bytesToAlign = ((size_t)m_stackStartPtr) % m_alignBytes;
	m_startPtrOffset = bytesToAlign;
	FreeTo(0);

	m_totalByteSize = _stackSizeBytes;
	m_growIfFull = _growIfFull;
	m_allocateIfFull = _allocateIfFull;
	m_reserveMutex = SDL_CreateMutex();
}

StackAllocator_SingleBuffer::~StackAllocator_SingleBuffer()
{
	free(m_stackStartPtr);
}

void* StackAllocator_SingleBuffer::Reserve(size_t _nBytes)
{
	void* pReservedMemory = NULL;

	//	Lock the mutex
	if (SDL_LockMutex(m_reserveMutex) == 0)
	{
		if ((m_currentStackIndex - m_startPtrOffset) + _nBytes <= m_totalByteSize)
		{
			pReservedMemory = &m_stackStartPtr[m_currentStackIndex];
			m_currentStackIndex += _nBytes;
		}
		//else if (m_growIfFull)
		//{
		//	// TODO Grow the stack, if possible.
		//}
		//else if (m_allocateIfFull)
		//{
		//	// TODO Allocate using regular new and have the stack allocator keep the pointers for future deleting?
		//}
		else
			throw std::string("Out of memory!");
		// TODO Throw exception / set error flag (or simply return the NULL pointer?) because there was an attempt to use the stack beyond its capacity.
		
		//	Unlock
		SDL_UnlockMutex(m_reserveMutex);
	}
	return pReservedMemory;
}

size_t StackAllocator_SingleBuffer::GetTop()
{
	return m_currentStackIndex;
}

void StackAllocator_SingleBuffer::FreeTo(size_t _index)
{
	m_currentStackIndex = m_startPtrOffset + _index;
}