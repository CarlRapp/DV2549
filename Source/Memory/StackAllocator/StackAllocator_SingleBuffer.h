#ifndef STACKALLOCATOR_SINGLEBUFFER_H
#define STACKALLOCATOR_SINGLEBUFFER_H

#include "IStackAllocator.h"

#include <SDL/SDL.h>
#include <atomic>

namespace Memory
{
	class DECLSPEC StackAllocator_SingleBuffer : public IStackAllocator
	{
	public:
		StackAllocator_SingleBuffer(size_t _stackSizeBytes, size_t _stackAlignment = 64, bool _growIfFull = false, bool _allocateIfFull = false) throw();
		~StackAllocator_SingleBuffer();

		bool HasPointer(void* _inPtr);
		void* Reserve(size_t _nBytes);
		size_t GetTop();
		void FreeTo(size_t _index);

	private:
		std::atomic<size_t>	m_currentStackIndex;
		size_t	m_startPtrOffset;
		size_t	m_totalByteSize;
		size_t	m_alignBytes;
		size_t	m_alignBitmask;

		bool	m_growIfFull;
		bool	m_allocateIfFull;
		
		SDL_mutex* m_reserveMutex;
		char*	m_stackStartPtr;
	};
}
#endif