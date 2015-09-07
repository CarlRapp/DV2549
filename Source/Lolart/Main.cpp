#include "MemoryManager.h"
#include "Items.h"

MemoryManager gMemoryManager; // Memory Manager, global variable

// void* operator new(size_t size)
// {
// 	return gMemoryManager.Allocate(size);
// }
// 
// void* operator new[](size_t size)
// {
// 	return  gMemoryManager.Allocate(size);
// }
// 
// void operator delete(void* pointerToDelete)
// {
// 	gMemoryManager.Free(pointerToDelete);
// }
// 
// void operator delete[](void* arrayToDelete)
// {
// 	gMemoryManager.Free(arrayToDelete);
// }

MyItem* myItem1;
MyItem* myItem2;

int main(int argc, int* argp)
{
	gMemoryManager.SetSize(3, sizeof(MyItem));
	myItem1 = new MyItem;
	myItem2 = new MyItem;
	myItem2 = new MyItem;
	myItem2 = new MyItem;
	delete myItem2;

	return 1;
}