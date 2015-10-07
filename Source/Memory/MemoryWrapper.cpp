#include "MemoryWrapper.h"
#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"
#include <assert.h>
#include <thread>

using namespace Memory;


//static MemoryWrapper* m_instance;

#pragma region Constructor/Destructor and GetInstance
MemoryWrapper::MemoryWrapper()
{
}
MemoryWrapper::~MemoryWrapper()
{
}

MemoryWrapper* MemoryWrapper::GetInstance()
{
	static MemoryWrapper* m_instance = new MemoryWrapper();
	return m_instance;
}

#pragma endregion


PoolManager* MemoryWrapper::GetPoolManager()
{
	std::thread::id tid = std::this_thread::get_id();

	if (m_poolmgrs.find(tid) != m_poolmgrs.end())
	{
		return m_poolmgrs[tid];
	}
	else
	{
		m_poolmgrs[tid] = new PoolManager();
	}
	return m_poolmgrs[tid];
}

void MemoryWrapper::DeletePoolManager()
{
	std::thread::id tid = std::this_thread::get_id();
	if (m_poolmgrs.find(tid) != m_poolmgrs.end())
	{
		delete m_poolmgrs[tid];
		m_poolmgrs.erase(tid);
	}
}

void MemoryWrapper::CreateGlobalStack(size_t _size, size_t _alignment)
{
	if (m_globalFrameStack)
	{
		for (int n = 0; n < m_oneFrameStacks.size(); ++n)
			if (m_oneFrameStacks[n] == m_globalFrameStack)
			{
				m_oneFrameStacks.erase(m_oneFrameStacks.begin() + n);
				break;
			}

		delete m_globalFrameStack;
		m_globalFrameStack = 0;
	}
	m_globalFrameStack = new Memory::StackAllocator_SingleBuffer(_size, _alignment);
	m_oneFrameStacks.push_back(m_globalFrameStack);
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

