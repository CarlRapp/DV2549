#ifndef STACKALLOCATOR_SINGLEBUFFER_H
#define STACKALLOCATOR_SINGLEBUFFER_H

#include "IStackAllocator.h"

#include <SDL/SDL.h>

namespace Memory
{
	class DECLSPEC StackAllocator_SingleBuffer : public IStackAllocator
	{
	public:
		StackAllocator_SingleBuffer(size_t _stackSizeBytes, size_t _stackAlignment = 64, bool _growIfFull = false, bool _allocateIfFull = false) throw();
		~StackAllocator_SingleBuffer();

		void* Reserve(size_t _nBytes);
		size_t GetTop();
		void FreeTo(size_t _index);

	private:
		char*	m_stackStartPtr;
		size_t	m_startPtrOffset;

		size_t	m_totalByteSize;
		size_t	m_currentStackIndex;
		size_t	m_alignBytes;

		bool	m_growIfFull;
		bool	m_allocateIfFull;
	};
}
#endif