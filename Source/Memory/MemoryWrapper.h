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
#include <thread>

#include "PoolAllocator/PoolManager.h"
#include "StackAllocator/IStackAllocator.h"




namespace Memory
{

	class DECLSPEC MemoryWrapper
	{

	private:
		MemoryWrapper();

		std::map<std::thread::id, PoolManager*> m_poolmgrs;

	public:
		~MemoryWrapper();
		static MemoryWrapper* GetInstance();

		/*
			C* a =	PNEW(C, 1, memoryWrapper);
					PNEW(Class , >1 if array , memorywrapper instance )
		*/

		PoolManager* GetPoolManager();
		void DeletePoolManager();



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