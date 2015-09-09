#pragma once
#include <vector>

class AllocatorInterface
{
private:

	unsigned int m_memPointer = 0;
	unsigned int m_maxMemory = 0;
	unsigned int m_memSlotSize = 0;

	void* m_memory = nullptr;

	std::vector<void*> m_memPoolList;
	std::vector<bool> m_memSlotsTaken;

public:

	virtual void* Allocate(size_t) = 0;
	virtual void Free(void*) = 0;
	virtual void SetSize(unsigned int _items, size_t _size) = 0;
};
