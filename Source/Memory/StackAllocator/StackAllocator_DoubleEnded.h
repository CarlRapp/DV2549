#ifndef STACKALLOCATOR_DOUBLEENDED_H
#define STACKALLOCATOR_DOUBLEENDED_H

#include "IStackAllocator.h"

#include <SDL/SDL.h>
#include <atomic>

namespace Memory
{
	class DECLSPEC StackAllocator_DoubleEnded : public IStackAllocator
	{
	public:
		StackAllocator_DoubleEnded(size_t _stackSizeBytes, size_t _stackAlignment = 64);
		~StackAllocator_DoubleEnded();

		bool HasPointer(void* _inPtr);
		void* Reserve(size_t _nBytes);
		void FreeTo(size_t _index);
		void* ReserveLeft(size_t _nBytes, size_t _currentRight);
		void* ReserveRight(size_t _nBytes, size_t _currentLeft);

	private:


	private:

		//	The left and right index counter
		std::atomic<size_t>	m_currentLeftIndex;
		std::atomic<size_t>	m_currentRightIndex;
		std::atomic<size_t> m_currentSide;

		//	Data for the stack
		size_t m_stackByteSize;
		size_t m_dataAlignment;
		

		//	Pointer data
		size_t m_stackPtrOffset;
		char* m_stackPtr;
	};
}
#endif