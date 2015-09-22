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
#include <map>
#include <shared_mutex>


#include "PoolAllocator/PoolAllocator.h"
#include "StackAllocator/IStackAllocator.h"



namespace Memory
{

	class DECLSPEC MemoryWrapper
	{

	private:
		MemoryWrapper();

		std::map<size_t, std::vector<PoolAllocator*>>* m_poolMap;
		std::shared_timed_mutex* m_mutex;

	public:
		~MemoryWrapper();
		static MemoryWrapper* GetInstance();

		/*
			C* a =	PNEW(C, 1, memoryWrapper);
					PNEW(Class , >1 if array , memorywrapper instance )
		*/

		//New and delete macros for arrays
#define NEW_ARRAY( _type, _size, _wrapper )		static_cast<_type*>(_wrapper->pnew(sizeof(_type)*_size));
#define DELETE_ARRAY( _type, _pointer, _size, _wrapper ) _wrapper->pdelete(static_cast<void*>(_pointer), sizeof(_type)*_size);

		//New and delete macros for single target
#define NEW( _type, _wrapper )		static_cast<_type*>(_wrapper->pnew(sizeof(_type)));
#define DELETE( _type, _pointer, _wrapper ) _wrapper->pdelete(static_cast<void*>(_pointer), sizeof(_type));

		/*
		pnew Usage:

			Class A*			= static_cast<Class*>(pnew(sizeof(Class)))
			Class A[arraySize]	= static_cast<Class*>(pnew(sizeof(Class)*arraySize)) 
		*/
		void*	pnew(size_t _size);
		/*
		pdelete Usage :

			pdelete(static_cast<void*>(p), sizeof(Class))
			pdelete(static_cast<void*>(p), sizeof(Class)*arraySize)

		*/
		void	pdelete(void*, size_t _size);

		void CreatePool(unsigned int _items, size_t _size);

		void ClearAllPools();

#ifdef MEMORY_DEBUG
		void PrintPoolsByteLevel();
		void PrintPoolsPoolLevel();
#endif



		/*	Stack implementations	*/
	public:
		/*
			Creates the global stack with the selected settings
		*/
		void	CreateGlobalStack(size_t _size, size_t _alignment);

		/*
		Usage:
			Allocate objects with a lifespan of one frame or less.
		Note:
			Stack will get reset after each frame. Data should not be put on the stack if it lives longer than this.
		*/
		IStackAllocator*	GetGlobalStack();

		/*
		Usage:
			Call this to create a new stack for a custom task. Otherwise use the global
		Note:
			This stack will be removed by the memory wrapper!
		*/
		IStackAllocator*	CreateStack(size_t _size, size_t _alignment);

		/*
		Usage:
			Resets all stacks
		*/
		void	ResetStacks();

	private:
		std::vector<IStackAllocator*>	m_oneFrameStacks;
	};
}

#endif