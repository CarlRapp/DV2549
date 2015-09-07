#ifndef MEMORY_H
#define MEMORY_H

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#endif
#else
#endif

#include <SDL/SDL.h>
#include <vector>
#include <string>

namespace Memory
{
	class DECLSPEC MemoryWrapper
	{
	public:
		~MemoryWrapper();
		static MemoryWrapper& GetInstance();



	private:
		MemoryWrapper();
	};
}

#endif