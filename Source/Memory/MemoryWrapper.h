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


#include "PoolAllocator/PoolAllocator.h"



namespace Memory
{

	class DECLSPEC MemoryWrapper
	{

	private:
		MemoryWrapper();

		void* pnew(size_t _size);
		void pdelete(void*, size_t _size);


		std::map<size_t, std::vector<PoolAllocator*>>* m_PoolMap;

	public:
		~MemoryWrapper();
		static MemoryWrapper& GetInstance();
		void DestoryInstance();

		template <typename T>
		T* PNew()
		{
			return static_cast<T*>(pnew(sizeof(T)));
		}

		template <typename T>
		void PDelete(T* _pointer)
		{
			pdelete(static_cast<void*>(_pointer), sizeof(T));
		}

		//for array
		template <typename T>
		T* PNew(size_t _size)
		{
			return static_cast<T*>(pnew(sizeof(T)*_size));
		}

		//for array
		template <typename T>
		void PDelete(T*,size_t _size)
		{
			pdelete(static_cast<void*>(T), sizeof(T)*_size);
		}

		void CreatePool(unsigned int _items, size_t _size);

		void ClearAllPools();

		void PrintPools();

	};
}

#endif