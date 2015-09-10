#include "StackAllocator_SingleBuffer.h"

using namespace Memory;

StackAllocator_SingleBuffer::StackAllocator_SingleBuffer(size_t _stackSizeBytes, size_t _pointerAlignment = 4, bool _growIfFull = false, bool _allocateIfFull = false)
	: m_stackStartPtr(0), m_currentStackIndex(0)
{
	m_stackStartPtr = (char*)malloc(_stackSizeBytes);
	// TODO Calculate alignment on pointer hurp durp
	m_totalByteSize += _stackSizeBytes; // TODO DONT FORGET TO ADD ALIGNMENT HERE HURP DURP
	m_growIfFull = _growIfFull;
	m_allocateIfFull = _allocateIfFull;
}

StackAllocator_SingleBuffer::~StackAllocator_SingleBuffer()
{
	free(m_stackStartPtr);
}

void* StackAllocator_SingleBuffer::Reserve(size_t _nBytes)
{
	void* pReservedMemory = NULL;

	if (m_currentStackIndex + _nBytes <= m_totalByteSize)
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
	//else
		// TODO Throw exception / set error flag (or simply return the NULL pointer?) because there was an attempt to use the stack beyond its capacity.

	return pReservedMemory;
}

size_t StackAllocator_SingleBuffer::GetTop()
{
	return m_currentStackIndex;
}

void StackAllocator_SingleBuffer::FreeTo(size_t _index)
{
	m_currentStackIndex = _index;
}