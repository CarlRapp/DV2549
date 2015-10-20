#include "StackAllocator_DoubleEnded.h"
#include <iostream>
#include <assert.h>

using namespace Memory;

StackAllocator_DoubleEnded::StackAllocator_DoubleEnded(size_t _stackSizeBytes, size_t _stackAlignment)
	: m_dataAlignment(_stackAlignment), m_stackByteSize(_stackSizeBytes)
{
	//	Check so alignment is greater than one
	assert(m_dataAlignment > 1);

	//	Allocate the number of requested bytes plus requested alignment minus on
	//	to make sure that the amount of requested bytes can be used. 
	//	(Worst case scenario, m_alignBytes-1 needs to be used to 'move' the pointer)
	m_stackPtr = (char*)malloc(_stackSizeBytes + (_stackAlignment - 1));

	//	Check so we got a valid pointer
	assert(m_stackPtr != 0);

	//	Alignment must be a power of two
	assert(!(m_dataAlignment & (m_dataAlignment - 1)));

	//	Calculate how much to align the start of the pointer
	size_t bytesToAlign = ((size_t)m_stackPtrOffset) % m_dataAlignment;
	m_stackPtrOffset = bytesToAlign;

	FreeTo(0);

	m_currentSide.store(0);
}

StackAllocator_DoubleEnded::~StackAllocator_DoubleEnded()
{
	free(m_stackPtr);
}

bool Memory::StackAllocator_DoubleEnded::HasPointer(void * _inPtr)
{
	size_t ptrValue = (size_t)_inPtr;
	size_t stackBlockPtr = (size_t)m_stackPtr + m_stackPtrOffset;

	if (stackBlockPtr <= ptrValue && ptrValue < stackBlockPtr + m_stackByteSize)
		return true;

	return false;
}

//This method will return a NULL pointer if the stack can' allocated the requested amount of bytes.
void* StackAllocator_DoubleEnded::Reserve(size_t _nBytes)
{
	void* returnPtr = 0;

	//	Calculate padding
	size_t bytesToReserve = (_nBytes + m_dataAlignment - 1) & ~(m_dataAlignment - 1);
	size_t currentSide = m_currentSide.fetch_add(1, std::memory_order_relaxed);


//	if (currentSide % 2 == 0)
	if (currentSide & 1 == 0)
		returnPtr = ReserveLeft(bytesToReserve, m_currentRightIndex.load());
	else
		returnPtr = ReserveRight(bytesToReserve, m_currentLeftIndex.load());

	return returnPtr;
}


void* StackAllocator_DoubleEnded::ReserveLeft(size_t _nBytes, size_t _currentRight)
{
	//	Fetch the current stack index and then increment it
	//	with the requested bytes together with additional padding
	size_t preIncrementIndex = m_currentLeftIndex.fetch_add(_nBytes, std::memory_order_relaxed);

	//	Check so the index is within memory bounds
	if (preIncrementIndex + _nBytes <= _currentRight)
		return m_stackPtr + preIncrementIndex;

	return 0;
}
void* StackAllocator_DoubleEnded::ReserveRight(size_t _nBytes, size_t _currentLeft)
{
	//	Fetch the current stack index and then increment it
	//	with the requested bytes together with additional padding
	size_t preIncrementIndex = m_currentRightIndex.fetch_sub(_nBytes, std::memory_order_relaxed);

	//	Check so the index is within memory bounds
	if (preIncrementIndex - _nBytes >= _currentLeft)
		return m_stackPtr + preIncrementIndex - _nBytes;

	return 0;
}

void StackAllocator_DoubleEnded::FreeTo(size_t _index)
{
	m_currentLeftIndex.store(m_stackPtrOffset + _index);
	m_currentRightIndex.store(m_stackByteSize + m_stackPtrOffset - _index);
}