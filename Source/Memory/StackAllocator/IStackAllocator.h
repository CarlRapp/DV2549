#ifndef ISTACKALLOCATOR_H
#define ISTACKALLOCATOR_H

#include <SDL/SDL.h>

#include <iostream>
#include <string>
#include <initializer_list>

namespace Memory
{
	class DECLSPEC IStackAllocator
	{
	public:
		// Reserves a portion of the stack's memory, starting at the stack top, for placing data in using "placement new".
		
		virtual void FreeTo(size_t _index) = 0;

		virtual void* Reserve(size_t _nBytes) = 0;
		
		virtual bool HasPointer(void* _inPtr) = 0;

		template<typename Type, typename... Args>
		Type* Push(Args... _inArg);
	};

	template<typename Type, typename... Args>
	Type* IStackAllocator::Push(Args... _inArgs)
	{
		return new (Reserve(sizeof(Type))) Type(std::forward<Args>(_inArgs)...);
	}
}
#endif