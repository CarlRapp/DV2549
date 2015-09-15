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
		
		
		virtual void* Reserve(size_t _nBytes) = 0;

		template<typename Type, typename... Args>
		Type* Push(Args... _inArg);
		
		virtual bool HasPointer(void* _inPtr) = 0;
	};

	template<typename Type, typename... Args>
	Type* IStackAllocator::Push(Args... _inArgs)
	{
		void* newAddress = Reserve(sizeof(Type));
		Type* newObject = new (newAddress) Type(std::forward<Args>(_inArgs)...);

		return newObject;
	}
}
#endif