#pragma once

extern MemoryManager gMemoryManager;

struct MyItem
{
	inline void* operator new(size_t size)
	{
		return gMemoryManager.Allocate(size);
	}

// 	inline void* operator new[](size_t size)
// 	{
// 		return  gMemoryManager.Allocate(size);
// 	}

	inline void operator delete(void* pointerToDelete)
	{
		gMemoryManager.Free(pointerToDelete);
	}

// 	inline void operator delete[](void* arrayToDelete)
// 	{
// 		gMemoryManager.Free(arrayToDelete);
// 	}

	char A = 'a';
	char B = 'b';
	char C = 'c';
};