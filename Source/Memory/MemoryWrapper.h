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

		void* PNew(size_t _size);
		void PDelete(void*, size_t _size);


		std::map<size_t, std::vector<PoolAllocator*>>* m_PoolMap;

	public:
		~MemoryWrapper();
		static MemoryWrapper& GetInstance();
		void DestoryInstance();

		template <typename T>
		T* PNew(T)
		{
			return static_cast<T*>(pnew(sizeof(T)));
		}

		template <typename T>
		void PDelete(T*)
		{
			pdelete(static_cast<void*>(T), sizeof(T));
		}

		void CreatePool(unsigned int _items, size_t _size);

		void ClearAllPools();

	};
}

#endif