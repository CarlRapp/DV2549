#ifndef POOLTEST_H
#define POOLTEST_H

#include <atomic>
#include <string>
#include <SDL/SDL.h>
#include <vector>

#include "Memory/MemoryWrapper.h"
#include "PoolAllocator/PoolAllocator.h"

#pragma region VerifyMTPool()
unsigned int verifyMTPool_BufferSize;
unsigned int verifyMTPool_DataTypeSize;
unsigned int verifyMTPool_NumThreads;
PoolAllocator* verifyMTPool_Buffer;
std::atomic_bool verifyMTPool_Ready;
std::vector<size_t>* verifyMTPool_MemoryPointers;

int VerifyMTPool_Thread(void* _ptr)
{
	/* Calculate all settings variables */
	unsigned int numDataTypesPerThread = (verifyMTPool_BufferSize / verifyMTPool_DataTypeSize) / verifyMTPool_NumThreads;
	unsigned int id = *static_cast<unsigned int*>(_ptr);
	verifyMTPool_MemoryPointers[id].resize(numDataTypesPerThread);

	/* Thread is stuck in a loop until main thread tells it to work */
	while (!verifyMTPool_Ready) {}

	/* Allocate memory and save pointers in a list for later verification */
	for (unsigned int i = 0; i < numDataTypesPerThread; ++i)
		verifyMTPool_MemoryPointers[id][i] = reinterpret_cast<size_t>(verifyMTPool_Buffer->Allocate());

	/* Free memory */
	delete _ptr;

	return 0;
}

void VerifyMTPool(unsigned int _numAllocations, unsigned int _dataTypeSize, unsigned int _numThreads)
{
	verifyMTPool_BufferSize = _numAllocations * _dataTypeSize;
	verifyMTPool_DataTypeSize = _dataTypeSize;
	verifyMTPool_NumThreads = _numThreads;

	/* Create a memory buffer */
	verifyMTPool_Buffer = 0;
	try
	{
		verifyMTPool_Buffer = new PoolAllocator(_numAllocations, _dataTypeSize);
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	/* Threads needs to start working at the same time, so the ready variable is set to false */
	verifyMTPool_Ready = false;

	/* Initialize list of memory pointers */
	verifyMTPool_MemoryPointers = new std::vector<size_t>[_numThreads];

	SDL_Thread** threads = new SDL_Thread*[_numThreads];
	/* Create all threads */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "Thread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(VerifyMTPool_Thread, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create thread %i\n", i);
			abort();
		}
	}

	/* Tell the threads to start working */
	verifyMTPool_Ready = true;

	/* Wait for all threads to finish working */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		int threadReturnValue;
		SDL_WaitThread(threads[i], &threadReturnValue);

		/* Check for failure when thread is being destroyed */
		if (threadReturnValue != 0)
		{
			printf("Something went wrong when destroying thread %i (Thread returns %i)\n", i, threadReturnValue);
			abort();
		}
	}

	bool errors = false;
	/* Verify if the memory addresses are okay */
	for (unsigned int threadIndex = 0; threadIndex < _numThreads; ++threadIndex)
	{
		for (unsigned int memoryIndex = 0; memoryIndex < verifyMTPool_MemoryPointers[threadIndex].size(); ++memoryIndex)
		{
			size_t memoryAddress = verifyMTPool_MemoryPointers[threadIndex][memoryIndex];

			/* TODO: Is memory within buffer? */

			/* Compare each memory address to all other memory adresses */
			for (unsigned int i = 0; i < _numThreads; ++i)
			{
				for (unsigned int j = 0; j < verifyMTPool_MemoryPointers[threadIndex].size(); ++j)
				{
					/* Ignore index if it is the same index that we are comparing it to */
					if (i == threadIndex && j == memoryIndex)
						continue;

					/* If memory address exists in more than one place, print error */
					if (memoryAddress == verifyMTPool_MemoryPointers[i][j])
					{
						printf("Memory has been found in more than one place (%iz). Locations: (Thread: %i, Memory Index: %i) and (Thread: %i, Memory Index: %i)\n",
							(int)memoryAddress,
							threadIndex,
							memoryIndex,
							i,
							j);
						errors = true;
					}
				}
			}
		}
	}

	printf("Results from VerifyMTPool():\n");
	printf("Number of Threads: %i\n", _numThreads);
	printf("Buffer Size: %i\n", verifyMTPool_BufferSize);
	printf("Data Type Size: %i\n", verifyMTPool_DataTypeSize);
	printf("Number of Allocated Data Types per Thread: %i\n", ((verifyMTPool_BufferSize / verifyMTPool_DataTypeSize) / verifyMTPool_NumThreads));
	if (errors)
		printf("Conclusion: Memory IS NOT being allocated perfectly in a multi-threaded environment\n");
	else
		printf("Conclusion: Memory is being allocated perfectly in a multi-threaded environment\n");
	printf("-------------------------------\n\n");

	/* Free all memory */
	delete[] threads;
	delete[] verifyMTPool_MemoryPointers;
	delete verifyMTPool_Buffer;
}
#pragma endregion

#pragma region MeasureSQPool()
unsigned int measureSQPool_BufferSize;
unsigned int measureSQPool_DataTypeSize;
PoolAllocator* measureSQPool_Buffer;

void MeasureSQPool(unsigned int _numAllocations, unsigned int _dataTypeSize)
{
	measureSQPool_BufferSize = _numAllocations * _dataTypeSize;
	measureSQPool_DataTypeSize = _dataTypeSize;

	/* Create a memory buffer */
	measureSQPool_Buffer = 0;
	try
	{
		measureSQPool_Buffer = new PoolAllocator(_numAllocations, _dataTypeSize);
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	/* Create two arrays of pointers, used for storing allocations */
	size_t* poolPointers = new size_t[_numAllocations]; // This array is actually not used for anything. Exists to even out the overhead performance
	size_t* defaultAllocatorPointers = new size_t[_numAllocations];

	/* Allocate pool memory */
	Uint32 start = SDL_GetTicks();
	for (unsigned int i = 0; i < _numAllocations; ++i)
	{
		poolPointers[i] = reinterpret_cast<size_t>(measureSQPool_Buffer->Allocate());
	}
	Uint32 poolAllocTime = (SDL_GetTicks() - start);

	/* Free pool memory */
	start = SDL_GetTicks();
	for (unsigned int i = 0; i < _numAllocations; ++i)
	{
		measureSQPool_Buffer->Free(reinterpret_cast<void*>(poolPointers[i]));
	}
	Uint32 poolFreeTime = (SDL_GetTicks() - start);

	/* Allocate default allocator memory */
	start = SDL_GetTicks();
	for (unsigned int i = 0; i < _numAllocations; ++i)
	{
		defaultAllocatorPointers[i] = reinterpret_cast<size_t>(malloc(measureSQPool_DataTypeSize));
	}
	Uint32 defaultAllocatorAllocTime = (SDL_GetTicks() - start);

	/* Free default allocator memory */
	start = SDL_GetTicks();
	for (unsigned int i = 0; i < _numAllocations; ++i)
	{
		free(reinterpret_cast<void*>(defaultAllocatorPointers[i]));
	}
	Uint32 defaultAllocatorFreeTime = (SDL_GetTicks() - start);

	printf("Results from MeasureSQPool():\n");
	printf("Buffer Size: %i\n", measureSQPool_BufferSize);
	printf("Data Type Size: %i\n", measureSQPool_DataTypeSize);
	printf("Number of Allocated Data Types: %i\n\n", _numAllocations);
	printf("Statistics:\n");
	printf("Sequential Pool Allocation Time: %ims\n", poolAllocTime);
	printf("Sequential Default Allocator Allocation Time: %ims\n", defaultAllocatorAllocTime);
	printf("Sequential Pool Free Time: %ims\n", poolFreeTime);
	printf("Sequential Default Allocator Free Time: %ims\n", defaultAllocatorFreeTime);
	printf("-------------------------------\n\n");

	/* Free memory */
	delete[] poolPointers;
	delete[] defaultAllocatorPointers;
	delete measureSQPool_Buffer;
}
#pragma endregion

#pragma region MeasureMTPool()
unsigned int measureMTPool_BufferSize;
unsigned int measureMTPool_DataTypeSize;
unsigned int measureMTPool_NumThreads;
PoolAllocator* measureMTPool_Buffer;
std::atomic_bool measureMTPool_Ready;
std::atomic_uint measureMTPool_FinishedPoolThreads;
std::atomic_uint measureMTPool_FinishedDefaultThreads;
size_t* measureMTPool_PoolPointers;
size_t* measureMTPool_DefaultAllocatorPointers;
Uint32 measureMTPool_StartTime;
Uint32 measureMTPool_PoolAllocationTime;
Uint32 measureMTPool_PoolFreeTime;
Uint32 measureMTPool_DefaultAllocationTime;
Uint32 measureMTPool_DefaultFreeTime;

int measureMTPool_ThreadPoolAlloc(void* _ptr)
{
	const unsigned int id = *static_cast<unsigned int*>(_ptr);
	const unsigned int numAllocations = (measureMTPool_BufferSize / measureMTPool_DataTypeSize) / measureMTPool_NumThreads;
	const unsigned int startIndex = (id * numAllocations);

	while (!measureMTPool_Ready) {}

	/* Allocations */
	for (unsigned int i = 0; i < numAllocations; ++i)
	{
		measureMTPool_PoolPointers[startIndex + i] = reinterpret_cast<size_t>(measureMTPool_Buffer->Allocate());
	}

	/* Thread is done, increase variable */
	++measureMTPool_FinishedPoolThreads;

	/* If this is the last thread to finish, measure time */
	if (measureMTPool_FinishedPoolThreads == measureMTPool_NumThreads)
		measureMTPool_PoolAllocationTime = SDL_GetTicks() - measureMTPool_StartTime;

	delete _ptr;

	return 0;
}

int measureMTPool_ThreadPoolFree(void* _ptr)
{
	const unsigned int id = *static_cast<unsigned int*>(_ptr);
	const unsigned int numFrees = (measureMTPool_BufferSize / measureMTPool_DataTypeSize) / measureMTPool_NumThreads;
	const unsigned int startIndex = (id * numFrees);

	while (!measureMTPool_Ready) {}

	/* Free memory */
	for (unsigned int i = 0; i < numFrees; ++i)
	{
		measureMTPool_Buffer->Free(reinterpret_cast<void*>(measureMTPool_PoolPointers[startIndex + i]));
		//delete(reinterpret_cast<int*>(measureMTPool_DefaultAllocatorPointers[startIndex + i]));
	}

	/* Thread is done, increase variable */
	++measureMTPool_FinishedPoolThreads;

	/* If this is the last thread to finish, measure time */
	if (measureMTPool_FinishedPoolThreads == measureMTPool_NumThreads)
		measureMTPool_PoolFreeTime = SDL_GetTicks() - measureMTPool_StartTime;

	delete _ptr;

	return 0;
}

int measureMTPool_ThreadDefaultAlloc(void* _ptr)
{
	const unsigned int id = *static_cast<unsigned int*>(_ptr);
	const unsigned int numAllocations = (measureMTPool_BufferSize / measureMTPool_DataTypeSize) / measureMTPool_NumThreads;
	const unsigned int startIndex = (id * numAllocations);

	while (!measureMTPool_Ready) {}

	/* Allocations */
	for (unsigned int i = 0; i < numAllocations; ++i)
	{
		measureMTPool_DefaultAllocatorPointers[startIndex + i] = reinterpret_cast<size_t>(malloc(measureMTPool_DataTypeSize));
	}

	/* Thread is done, increase variable */
	++measureMTPool_FinishedDefaultThreads;

	/* If this is the last thread to finish, measure time */
	if (measureMTPool_FinishedDefaultThreads == measureMTPool_NumThreads)
		measureMTPool_DefaultAllocationTime = SDL_GetTicks() - measureMTPool_StartTime;

	delete _ptr;

	return 0;
}

int measureMTPool_ThreadDefaultFree(void* _ptr)
{
	const unsigned int id = *static_cast<unsigned int*>(_ptr);
	const unsigned int numFrees = (measureMTPool_BufferSize / measureMTPool_DataTypeSize) / measureMTPool_NumThreads;
	const unsigned int startIndex = (id * numFrees);

	while (!measureMTPool_Ready) {}

	/* Free memory */
	for (unsigned int i = 0; i < numFrees; ++i)
	{
		free(reinterpret_cast<void*>(measureMTPool_DefaultAllocatorPointers[startIndex + i]));
		//delete(reinterpret_cast<int*>(measureMTPool_DefaultAllocatorPointers[startIndex + i]));
	}

	/* Thread is done, increase variable */
	++measureMTPool_FinishedDefaultThreads;

	/* If this is the last thread to finish, measure time */
	if (measureMTPool_FinishedDefaultThreads == measureMTPool_NumThreads)
		measureMTPool_DefaultFreeTime = SDL_GetTicks() - measureMTPool_StartTime;

	delete _ptr;

	return 0;
}

void MeasureMTPool(unsigned int _numAllocations, unsigned int _dataTypeSize, unsigned int _numThreads)
{
	measureMTPool_BufferSize = _numAllocations * _dataTypeSize;
	measureMTPool_DataTypeSize = _dataTypeSize;
	measureMTPool_NumThreads = _numThreads;

	/* Create a memory buffer */
	measureMTPool_Buffer = 0;
	try
	{
		measureMTPool_Buffer = new PoolAllocator(_numAllocations, _dataTypeSize);
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	/* Initialize two arrays of pointers, used for storing allocations */
	measureMTPool_PoolPointers = new size_t[_numAllocations]; // This array is actually not used for anything. Exists to even out the overhead performance
	measureMTPool_DefaultAllocatorPointers = new size_t[_numAllocations];

	/* Create thread array */
	SDL_Thread** threads = new SDL_Thread*[_numThreads];

#pragma region Pool Allocation Time
	measureMTPool_Ready = false;
	measureMTPool_FinishedPoolThreads = 0;

	/* Create threads for pool allocation */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "PoolThread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(measureMTPool_ThreadPoolAlloc, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create pool thread %i\n", i);
			abort();
		}
	}

	/* Begin measure time */
	measureMTPool_StartTime = SDL_GetTicks();

	/* Tell the threads to start allocating */
	measureMTPool_Ready = true;

	/* Wait for all threads to finish working */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		int threadReturnValue;
		SDL_WaitThread(threads[i], &threadReturnValue);

		/* Check for failure when thread is being destroyed */
		if (threadReturnValue != 0)
		{
			printf("Something went wrong when destroying thread %i (Thread returns %i)\n", i, threadReturnValue);
			abort();
		}
	}
#pragma endregion

#pragma region Default Allocator Allocation Time
	measureMTPool_Ready = false;
	measureMTPool_FinishedDefaultThreads = 0;

	/* Create threads for default allocator allocation */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "DefaultThread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(measureMTPool_ThreadDefaultAlloc, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create default thread %i\n", i);
			abort();
		}
	}

	/* Begin measure time */
	measureMTPool_StartTime = SDL_GetTicks();

	/* Tell the threads to start allocating */
	measureMTPool_Ready = true;

	/* Wait for all threads to finish working */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		int threadReturnValue;
		SDL_WaitThread(threads[i], &threadReturnValue);

		/* Check for failure when thread is being destroyed */
		if (threadReturnValue != 0)
		{
			printf("Something went wrong when destroying thread %i (Thread returns %i)\n", i, threadReturnValue);
			abort();
		}
	}
#pragma endregion

#pragma region Pool Free Time
	measureMTPool_Ready = false;
	measureMTPool_FinishedPoolThreads = 0;

	/* Create threads for freeing memory with pool allocator */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "PoolThread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(measureMTPool_ThreadPoolFree, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create pool thread %i\n", i);
			abort();
		}
	}

	/* Begin measure time */
	measureMTPool_StartTime = SDL_GetTicks();

	/* Tell the threads to start allocating */
	measureMTPool_Ready = true;

	/* Wait for all threads to finish working */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		int threadReturnValue;
		SDL_WaitThread(threads[i], &threadReturnValue);

		/* Check for failure when thread is being destroyed */
		if (threadReturnValue != 0)
		{
			printf("Something went wrong when destroying thread %i (Thread returns %i)\n", i, threadReturnValue);
			abort();
		}
	}
#pragma endregion

#pragma region Default Allocator Free Time
	measureMTPool_Ready = false;
	measureMTPool_FinishedDefaultThreads = 0;

	/* Create threads for freeing memory with default allocator */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "DefaultThread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(measureMTPool_ThreadDefaultFree, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create default thread %i\n", i);
			abort();
		}
	}

	/* Begin measure time */
	measureMTPool_StartTime = SDL_GetTicks();

	/* Tell the threads to start freeing memory */
	measureMTPool_Ready = true;

	/* Wait for all threads to finish working */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		int threadReturnValue;
		SDL_WaitThread(threads[i], &threadReturnValue);

		/* Check for failure when thread is being destroyed */
		if (threadReturnValue != 0)
		{
			printf("Something went wrong when destroying thread %i (Thread returns %i)\n", i, threadReturnValue);
			abort();
		}
	}
#pragma endregion

	printf("Results from MeasureMTPool():\n");
	printf("Number of Threads: %i\n", _numThreads);
	printf("Buffer Size: %i\n", measureMTPool_BufferSize);
	printf("Data Type Size: %i\n", measureMTPool_DataTypeSize);
	printf("Number of Allocated Data Types per Thread: %i\n\n", (_numAllocations / _numThreads));
	printf("Statistics:\n");
	printf("Multi-Threaded Pool Allocation Time: %ims\n", measureMTPool_PoolAllocationTime);
	printf("Multi-Threaded Default Allocator Allocation Time: %ims\n", measureMTPool_DefaultAllocationTime);
	printf("Multi-Threaded Pool Free Time: %ims\n", measureMTPool_PoolFreeTime);
	printf("Multi-Threaded Default Allocator Free Time: %ims\n", measureMTPool_DefaultFreeTime);
	printf("-------------------------------\n\n");

	/* Free memory */
	delete[] threads;
	delete[] measureMTPool_PoolPointers;
	delete[] measureMTPool_DefaultAllocatorPointers;
	delete measureMTPool_Buffer;
}
#pragma endregion

#endif
