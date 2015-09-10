#ifndef ISTACKALLOCATOR_H
#define ISTACKALLOCATOR_H

#include <SDL/SDL.h>

namespace Memory
{
	class DECLSPEC IStackAllocator
	{
	public:
		virtual void* Reserve(size_t _nBytes) = 0;

	};
}
#endif