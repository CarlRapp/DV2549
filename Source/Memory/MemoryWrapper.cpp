#include "MemoryWrapper.h"
#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"
#include <assert.h>

using namespace Memory;


//static MemoryWrapper* m_instance;

#pragma region Constructor/Destructor and GetInstance
MemoryWrapper::MemoryWrapper()
{
	m_poolMap = new std::map<size_t, std::vector<PoolAllocator*>>();
	m_mutex = new std::shared_timed_mutex;
}
MemoryWrapper::~MemoryWrapper()
{
	ClearAllPools();
	delete m_poolMap;
	delete m_mutex;
}

MemoryWrapper* MemoryWrapper::GetInstance()
{
	static MemoryWrapper* m_instance = new MemoryWrapper();
	return m_instance;
}

#pragma endregion

void* MemoryWrapper::pnew(size_t _size)
{

#ifdef MEMORY_PADDING
	size_t size = GetPaddedBlockSize(_size);
#else
	size_t size = _size;
#endif

	m_mutex->lock_shared();
#if MEMORY_DEBUG //&& !_DEBUG
	if (m_poolMap->find(size) == m_poolMap->end())
	{
		printf("No pool created for size %d\n", (int)_size);
		return nullptr;
	}
//#elif //_DEBUG
	assert(m_poolMap->find(size) != m_poolMap->end());
#endif

	auto list = &m_poolMap->at(size);
	m_mutex->unlock_shared();

	for (unsigned int i = 0; i < list->size(); ++i)
	{
		if (list->at(i)->HasFreeSlot())
		{
			return list->at(i)->Allocate();
		}
	}

#if MEMORY_DEBUG //&& !_DEBUG
	printf("All pools are full for size %d\n", (int)_size);
//#elif _DEBUG
	assert(0);
#endif
	return nullptr;
}

void MemoryWrapper::pdelete(void* _delete, size_t _size)
{
#ifdef MEMORY_PADDING
	size_t size = GetPaddedBlockSize(_size);
#else
	size_t size = _size;
#endif

	m_mutex->lock_shared();
#if MEMORY_DEBUG// && !_DEBUG
	if (m_poolMap->find(size) == m_poolMap->end())
	{
		printf("No pool created for size %d\n", (int)_size);
		return;
	}
//#elif _DEBUG
	assert(m_poolMap->find(size) != m_poolMap->end());
#endif

	auto list = &m_poolMap->at(size);
	m_mutex->unlock_shared();

	for (unsigned int i = 0; i < list->size(); ++i)
	{
		if (list->at(i)->IsInRange(_delete))
		{
			list->at(i)->Free(_delete);
			return;
		}
	}

#if MEMORY_DEBUG //&& !_DEBUG
	printf("No pool found to delete from\n");
//#elif _DEBUG
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
	m_mutex->lock();
	(*m_poolMap)[size].push_back(new PoolAllocator(_items, _size)); 
	m_mutex->unlock();
}

void MemoryWrapper::ClearAllPools()
{
	m_mutex->lock();
	for (auto it = m_poolMap->begin(); it != m_poolMap->end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); ++i)
		{
			delete it->second[i];
		}
	}
	m_poolMap->clear();
	m_mutex->unlock();
}

void MemoryWrapper::PrintPoolsByteLevel()
{
	for (auto it = m_poolMap->begin(); it != m_poolMap->end(); ++it)
	{
		printf("\nSIZE %d", (int)it->first);

		for (unsigned int i = 0; i < it->second.size(); ++i)
		{
			printf("\nPOOL %d\n", i);

			char* str = it->second[i]->GetPrint();
			unsigned int maxMem = it->second[i]->GetMaxMemory();
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
}

void MemoryWrapper::PrintPoolsPoolLevel()
{
	printf("\n_Pool Memory Print_\n");

	for (auto it = m_poolMap->begin(); it != m_poolMap->end(); ++it)
	{
		printf("\nSIZE %d", (int)it->first);

		for (unsigned int i = 0; i < it->second.size(); ++i)
		{
			printf("\n	POOL %d\n", i);

			printf("	Free slots: %d/%d", (int)it->second[i]->GetFreeSlots(), (int)it->second[i]->GetMaxSlots());
		}
	}

	printf("\n\n");
}

void MemoryWrapper::CreateGlobalStack(size_t _size, size_t _alignment)
{
	m_oneFrameStacks.push_back(new Memory::StackAllocator_SingleBuffer(_size, _alignment));
}

IStackAllocator * Memory::MemoryWrapper::GetGlobalStack()
{
	return m_oneFrameStacks[0];
}

IStackAllocator * Memory::MemoryWrapper::CreateStack(size_t _size, size_t _alignment)
{
	IStackAllocator* newStack = new Memory::StackAllocator_SingleBuffer(_size, _alignment);
	m_oneFrameStacks.push_back(newStack);
	return newStack;
}

void MemoryWrapper::ResetStacks()
{
	for (int n = 0; n < m_oneFrameStacks.size(); ++n)
		m_oneFrameStacks[n]->FreeTo(0);
}

