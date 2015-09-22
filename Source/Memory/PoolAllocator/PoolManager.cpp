#include "PoolManager.h"
#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"
#include <assert.h>

using namespace Memory;


//static MemoryWrapper* m_instance;

#pragma region Constructor/Destructor and GetInstance
PoolManager::PoolManager()
{
	//m_poolMap = new std::map<size_t, PoolAllocator*>();
	//m_mutex = new std::shared_timed_mutex;
}
PoolManager::~PoolManager()
{
	ClearAllPools();
	//delete m_poolMap;
	//delete m_mutex;
}

#pragma endregion

void* PoolManager::pnew(size_t _size)
{

	//#ifdef MEMORY_PADDING
	//	size_t size = GetPaddedBlockSize(_size);
	//#else
	//	size_t size = _size;
	//#endif

		//m_mutex->lock_shared();
#if MEMORY_DEBUG //&& !_DEBUG
	if (m_poolMap->find(_size) == m_poolMap->end())
	{
		printf("No pool created for size %d\n", (int)_size);
		return nullptr;
	}
	//#elif //_DEBUG
	assert(m_poolMap->find(size) != m_poolMap->end());
#endif

	PoolAllocator* pool = m_poolMap[_size];
	if (pool->HasFreeSlot())
		return pool->Allocate();


#if MEMORY_DEBUG //&& !_DEBUG
	printf("All pools are full for size %d\n", (int)_size);
	//#elif _DEBUG
	assert(0);
#endif
	return nullptr;
}

void PoolManager::pdelete(void* _delete, size_t _size)
{
//#ifdef MEMORY_PADDING
//	size_t size = GetPaddedBlockSize(_size);
//#else
//	size_t size = _size;
//#endif

	//m_mutex->lock_shared();
#if MEMORY_DEBUG// && !_DEBUG
	if (m_poolMap->find(_size) == m_poolMap->end())
	{
		printf("No pool created for size %d\n", (int)_size);
		return;
	}
	//#elif _DEBUG
	assert(m_poolMap->find(size) != m_poolMap->end());
#endif

	PoolAllocator* pool = m_poolMap[_size];
	//m_mutex->unlock_shared();

#if MEMORY_DEBUG //&& !_DEBUG
	if (!pool->IsInRange(_delete))
	{
		printf("Pointer out of range.\n");
		//#elif _DEBUG
		assert(0);
	}
#endif
	pool->Free(_delete);
	return;
}

void PoolManager::CreatePool(unsigned int _items, size_t _size)
{
//#ifdef MEMORY_PADDING
//	size_t size = GetPaddedBlockSize(_size);
//#else
//	size_t size = _size;
//#endif

#if MEMORY_DEBUG
	assert(m_poolMap.find(_size) == m_poolMap.end());
#endif

	//m_mutex->lock();
	m_poolMap[_size] = new PoolAllocator(_items, _size);
	//m_mutex->unlock();
}

void PoolManager::ClearAllPools()
{
	//m_mutex->lock();
	for (auto it = m_poolMap.begin(); it != m_poolMap.end(); ++it)
	{
		delete it->second;
	}
	m_poolMap.clear();
	//m_mutex->unlock();
}

void PoolManager::PrintPoolsByteLevel()
{
	for (auto it = m_poolMap.begin(); it != m_poolMap.end(); ++it)
	{
		printf("\nSIZE %d", (int)it->first);


		char* str = it->second->GetPrint();
		unsigned int maxMem = it->second->GetMaxMemory();
		unsigned int counter = 0;
		for (unsigned int k = 0; k < maxMem; k++)
		{
			counter++;
			if (str[k] != 0)
				printf("X");
			else
				printf("|");

			if (counter >= it->first)
			{
				printf(" ");
				counter = 0;
			}
		}
		
	}
}

void PoolManager::PrintPoolsPoolLevel()
{
	printf("\n_Pool Memory Print_\n");

	for (auto it = m_poolMap.begin(); it != m_poolMap.end(); ++it)
	{
		printf("\nSIZE %d", (int)it->first);

		printf("	Free slots: %d/%d", (int)it->second->GetFreeSlots(), (int)it->second->GetMaxSlots());
		
	}

	printf("\n\n");
}


