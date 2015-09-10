#ifndef ISTACKALLOCATOR_H
#define ISTACKALLOCATOR_H

#include <SDL/SDL.h>

namespace Memory
{
	class DECLSPEC IStackAllocator
	{
	public:
		virtual ~IStackAllocator() = 0;

		// Reserves a portion of the stack's memory, starting at the stack top, for placing data in using "placement new".
		virtual void* Reserve(size_t _nBytes) = 0;

		//virtual size_t GetTop() = 0;
		//virtual void FreeTo(size_t _index) = 0;
	};
}
#endif