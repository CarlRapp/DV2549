#ifndef POOLMANAGER_H
#define POOLMANAGER_H

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

#include "PoolAllocator.h"



namespace Memory
{

	class DECLSPEC PoolManager
	{

	private:

		std::map<size_t, PoolAllocator*> m_poolMap;

	public:
		PoolManager();
		~PoolManager();

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

	};
}

#endif