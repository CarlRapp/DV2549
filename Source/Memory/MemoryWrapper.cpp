#include "MemoryWrapper.h"
#include <assert.h>

using namespace Memory;


static MemoryWrapper* m_instance;

#pragma region Constructor/Destructor and GetInstance
MemoryWrapper::MemoryWrapper()
{
	m_PoolMap = new std::map<size_t, std::vector<PoolAllocator*>>();
}
MemoryWrapper::~MemoryWrapper()
{
	ClearAllPools();
	delete m_PoolMap;
}

MemoryWrapper& MemoryWrapper::GetInstance()
{
	m_instance = new MemoryWrapper();
	return *m_instance;
}

void MemoryWrapper::DestoryInstance()
{
	delete m_instance;
}

#pragma endregion

void* MemoryWrapper::PNew(size_t _size)
{

#ifdef MEMORY_PADDING
	size_t size = GetPaddedBlockSize(_size);
#else
	size_t size = _size;
#endif

#if MEMORY_DEBUG && !_DEBUG
	if (m_PoolMap.find(size) == m_PoolMap.end())
	{
		printf("No pool created for size %d\n", _size);
		return nullptr;
	}
#elif _DEBUG
	assert(m_PoolMap->find(size) != m_PoolMap->end());
#endif

	auto list = &m_PoolMap->at(size);

	for (unsigned int i = 0; i < list->size(); ++i)
	{
		if (list->at(i)->HasFreeSlot())
		{
			return list->at(i)->Allocate();
		}
	}

#if MEMORY_DEBUG && !_DEBUG
	printf("All pools are full for size %d\n", _size);
#elif _DEBUG
	assert(0);
#endif
	return nullptr;
}

void MemoryWrapper::PDelete(void* _delete, size_t _size)
{
#ifdef MEMORY_PADDING
	size_t size = GetPaddedBlockSize(_size);
#else
	size_t size = _size;
#endif

#if MEMORY_DEBUG && !_DEBUG
	if (m_PoolMap.find(size) == m_PoolMap.end())
	{
		printf("No pool created for size %d\n", _size);
		return nullptr;
	}
#elif _DEBUG
	assert(m_PoolMap->find(size) != m_PoolMap->end());
#endif

	auto list = &m_PoolMap->at(size);
	for (unsigned int i = 0; i < list->size(); ++i)
	{
		if (list->at(i)->IsInRange(_delete))
		{
			list->at(i)->Free(_delete);
			return;
		}
	}

#if MEMORY_DEBUG && !_DEBUG
	printf("No pool found to delete from\n");
#elif _DEBUG
	assert(0);
#endif

}

void MemoryWrapper::CreatePool(unsigned int _items, size_t _size)
{
#ifdef MEMORY_PADDING
	size_t size = GetPaddedBlockSize(_size);
#else
	size_t size = _size;
#endif
	m_PoolMap->at(size).push_back(new PoolAllocator(_items, _size));
}

void MemoryWrapper::ClearAllPools()
{
	for (auto it = m_PoolMap->begin(); it != m_PoolMap->end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); ++i)
		{
			delete it->second[i];
		}
	}
	m_PoolMap->clear();
}


