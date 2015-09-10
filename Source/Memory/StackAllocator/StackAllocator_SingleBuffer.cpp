#include "StackAllocator_SingleBuffer.h"

using namespace Memory;



StackAllocator_SingleBuffer::StackAllocator_SingleBuffer(size_t _stackSizeBytes, size_t _pointerAlignment)
	: m_stackStartPtr(0), m_currentStackIndex(0)
{
	m_stackStartPtr = (char*)malloc(_stackSizeBytes);
	//	Calculate alignment on pointer hurp durp
	m_totalByteSize += _stackSizeBytes; // DONT FORGET TO ADD ALIGNMENT HERE HURP DURP
}


StackAllocator_SingleBuffer::~StackAllocator_SingleBuffer()
{
}

void* StackAllocator_SingleBuffer::Reserve(size_t _nBytes)
{
	void* returnPtr = &m_stackStartPtr[m_currentStackIndex];
	m_currentStackIndex += _nBytes;
	return returnPtr;
}

size_t StackAllocator_SingleBuffer::GetTop()
{
	return m_currentStackIndex;
}

void StackAllocator_SingleBuffer::FreeTo(size_t _index)
{
	m_currentStackIndex = _index;
}