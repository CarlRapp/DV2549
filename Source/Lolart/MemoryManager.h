#pragma once
#include <vector>

class MemoryManager
{
private:
	unsigned int m_memPointer = 0;
	unsigned int m_maxMemory = 0;
	unsigned int m_memSlotSize = 0;
	void* m_memory = nullptr;
	std::vector<void*> m_memPoolList;
	std::vector<bool> m_memSlotsTaken;
public:
	MemoryManager();
	~MemoryManager();
	void* Allocate(size_t);
	void Free(void*);
	void SetSize(unsigned int _items, size_t _size);
};
