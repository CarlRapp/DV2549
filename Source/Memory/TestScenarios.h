#ifndef TESTSCENARIOS_H
#define TESTSCENARIOS_H

#include <atomic>
#include <string>
#include <SDL/SDL.h>
#include <vector>

#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"

#pragma region Stack

#pragma region VerifyMTStack()
unsigned int verifyMTStack_BufferSize;
unsigned int verifyMTStack_DataTypeSize;
unsigned int verifyMTStack_NumThreads;
Memory::StackAllocator_SingleBuffer* verifyMTStack_Buffer;
std::atomic_bool verifyMTStack_Ready;
std::vector<size_t>* verifyMTStack_MemoryPointers;

int VerifyMTStack_Thread(void* _ptr)
{
	/* Calculate all settings variables */
	unsigned int numDataTypesPerThread = (verifyMTStack_BufferSize / verifyMTStack_DataTypeSize) / verifyMTStack_NumThreads;
	unsigned int id = *static_cast<unsigned int*>(_ptr);
	verifyMTStack_MemoryPointers[id].resize(numDataTypesPerThread);

	/* Thread is stuck in a loop until main thread tells it to work */
	while (!verifyMTStack_Ready) {}

	/* Allocate memory and save pointers in a list for later verification */
	for (unsigned int i = 0; i < numDataTypesPerThread; ++i)
		verifyMTStack_MemoryPointers[id][i] = reinterpret_cast<size_t>(verifyMTStack_Buffer->Reserve(verifyMTStack_DataTypeSize));

	/* Free memory */
	delete _ptr;

	return 0;
}

void VerifyMTStack(unsigned int _numAllocations, unsigned int _dataTypeSize, unsigned int _numThreads, unsigned int _alignment)
{
	verifyMTStack_BufferSize = _numAllocations * _dataTypeSize;
	verifyMTStack_DataTypeSize = _dataTypeSize;
	verifyMTStack_NumThreads = _numThreads;

	/* Create a memory buffer */
	verifyMTStack_Buffer = 0;
	try
	{
		verifyMTStack_Buffer = new Memory::StackAllocator_SingleBuffer(verifyMTStack_BufferSize, _alignment);
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	/* Threads needs to start working at the same time, so the ready variable is set to false */
	verifyMTStack_Ready = false;

	/* Initialize list of memory pointers */
	verifyMTStack_MemoryPointers = new std::vector<size_t>[_numThreads];
	
	SDL_Thread** threads = new SDL_Thread*[_numThreads];
	/* Create all threads */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "Thread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(VerifyMTStack_Thread, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create thread %i\n", i);
			abort();
		}
	}

	/* Tell the threads to start working */
	verifyMTStack_Ready = true;

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
		for (unsigned int memoryIndex = 0; memoryIndex < verifyMTStack_MemoryPointers[threadIndex].size(); ++memoryIndex)
		{
			size_t memoryAddress = verifyMTStack_MemoryPointers[threadIndex][memoryIndex];

			/* TODO: Is memory within buffer? */

			/* Compare each memory address to all other memory adresses */
			for (unsigned int i = 0; i < _numThreads; ++i)
			{
				for (unsigned int j = 0; j < verifyMTStack_MemoryPointers[threadIndex].size(); ++j)
				{
					/* Ignore index if it is the same index that we are comparing it to */
					if (i == threadIndex && j == memoryIndex)
						continue;

					/* If memory address exists in more than one place, print error */
					if (memoryAddress == verifyMTStack_MemoryPointers[i][j])
					{
						printf("Memory has been found in more than one place (%iz). Locations: (Thread: %i, Memory Index: %i) and (Thread: %i, Memory Index: %i)\n", 
							memoryAddress,
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

	printf("Results from VerifyMTStack():\n");
	printf("Number of Threads: %i\n", _numThreads);
	printf("Buffer Size: %i\n", verifyMTStack_BufferSize);
	printf("Data Type Size: %i\n", verifyMTStack_DataTypeSize);
	printf("Number of Allocated Data Types per Thread: %i\n", ((verifyMTStack_BufferSize / verifyMTStack_DataTypeSize) / verifyMTStack_NumThreads));
	if (errors)
		printf("Conclusion: Memory IS NOT being allocated perfectly in a multi-threaded environment\n");
	else
		printf("Conclusion: Memory is being allocated perfectly in a multi-threaded environment\n");
	printf("-------------------------------\n\n");

	/* Free all memory */
	delete[] threads;
	delete[] verifyMTStack_MemoryPointers;
	delete verifyMTStack_Buffer;
}
#pragma endregion

#pragma region MeasureSQStack()
unsigned int measureSQStack_BufferSize;
unsigned int measureSQStack_DataTypeSize;
Memory::StackAllocator_SingleBuffer* measureSQStack_Buffer;

void MeasureSQStack(unsigned int _numAllocations, unsigned int _dataTypeSize, unsigned int _alignment)
{
	measureSQStack_BufferSize = _numAllocations * _dataTypeSize;
	measureSQStack_DataTypeSize = _dataTypeSize;

	/* Create a memory buffer */
	measureSQStack_Buffer = 0;
	try
	{
		measureSQStack_Buffer = new Memory::StackAllocator_SingleBuffer(measureSQStack_BufferSize, _alignment);
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	/* Create two arrays of pointers, used for storing allocations */
	size_t* stackPointers = new size_t[_numAllocations]; // This array is actually not used for anything. Exists to even out the overhead performance
	size_t* defaultAllocatorPointers = new size_t[_numAllocations];

	/* Allocate stack memory */
	Uint32 start = SDL_GetTicks();
	for (unsigned int i = 0; i < _numAllocations; ++i)
	{
		stackPointers[i] = reinterpret_cast<size_t>(measureSQStack_Buffer->Reserve(measureSQStack_DataTypeSize));
	}
	Uint32 stackAllocTime = (SDL_GetTicks() - start);

	/* Free stack memory */
	start = SDL_GetTicks();
	measureSQStack_Buffer->FreeTo(0);
	Uint32 stackFreeTime = (SDL_GetTicks() - start);

	/* Allocate default allocator memory */
	start = SDL_GetTicks();
	for (unsigned int i = 0; i < _numAllocations; ++i)
	{
		defaultAllocatorPointers[i] = reinterpret_cast<size_t>(malloc(measureSQStack_DataTypeSize));
		//defaultAllocatorPointers[i] = reinterpret_cast<size_t>(new int());
	}
	Uint32 defaultAllocatorAllocTime = (SDL_GetTicks() - start);

	/* Free default allocator memory */
	start = SDL_GetTicks();
	for (unsigned int i = 0; i < _numAllocations; ++i)
	{
		free(reinterpret_cast<void*>(defaultAllocatorPointers[i]));
		//delete(reinterpret_cast<int*>(defaultAllocatorPointers[i]));
	}
	Uint32 defaultAllocatorFreeTime = (SDL_GetTicks() - start);

	printf("Results from MeasureSQStack():\n");
	printf("Buffer Size: %i\n", measureSQStack_BufferSize);
	printf("Data Type Size: %i\n", measureSQStack_DataTypeSize);
	printf("Number of Allocated Data Types: %i\n\n", _numAllocations);
	printf("Statistics:\n");
	printf("Sequential Stack Allocation Time: %ims\n", stackAllocTime);
	printf("Sequential Default Allocator Allocation Time: %ims\n", defaultAllocatorAllocTime);
	printf("Sequential Stack Free Time: %ims\n", stackFreeTime);
	printf("Sequential Default Allocator Free Time: %ims\n", defaultAllocatorFreeTime);
	printf("-------------------------------\n\n");

	/* Free memory */
	delete[] stackPointers;
	delete[] defaultAllocatorPointers;
	delete measureSQStack_Buffer;
}
#pragma endregion

#pragma region MeasureMTStack()
unsigned int measureMTStack_BufferSize;
unsigned int measureMTStack_DataTypeSize;
unsigned int measureMTStack_NumThreads;
Memory::StackAllocator_SingleBuffer* measureMTStack_Buffer;
std::atomic_bool measureMTStack_Ready;
std::atomic_uint measureMTStack_FinishedStackThreads;
std::atomic_uint measureMTStack_FinishedDefaultThreads;
size_t* measureMTStack_StackPointers;
size_t* measureMTStack_DefaultAllocatorPointers;
Uint32 measureMTStack_StartTime;
Uint32 measureMTStack_StackAllocationTime;
Uint32 measureMTStack_StackFreeTime;
Uint32 measureMTStack_DefaultAllocationTime;
Uint32 measureMTStack_DefaultFreeTime;

int MeasureMTStack_ThreadStackAlloc(void* _ptr)
{
	const unsigned int id = *static_cast<unsigned int*>(_ptr);
	const unsigned int numAllocations = (measureMTStack_BufferSize / measureMTStack_DataTypeSize) / measureMTStack_NumThreads;
	const unsigned int startIndex = (id * numAllocations);

	while (!measureMTStack_Ready) { }

	/* Allocations */
	for (unsigned int i = 0; i < numAllocations; ++i)
	{
		measureMTStack_StackPointers[startIndex + i] = reinterpret_cast<size_t>(measureMTStack_Buffer->Reserve(measureMTStack_DataTypeSize));
	}

	/* Thread is done, increase variable */
	++measureMTStack_FinishedStackThreads;

	/* If this is the last thread to finish, measure time */
	if (measureMTStack_FinishedStackThreads == measureMTStack_NumThreads)
		measureMTStack_StackAllocationTime = SDL_GetTicks() - measureMTStack_StartTime;

	delete _ptr;

	return 0;
}

int MeasureMTStack_ThreadDefaultAlloc(void* _ptr)
{
	const unsigned int id = *static_cast<unsigned int*>(_ptr);
	const unsigned int numAllocations = (measureMTStack_BufferSize / measureMTStack_DataTypeSize) / measureMTStack_NumThreads;
	const unsigned int startIndex = (id * numAllocations);

	while (!measureMTStack_Ready) {}

	/* Allocations */
	for (unsigned int i = 0; i < numAllocations; ++i)
	{
		measureMTStack_DefaultAllocatorPointers[startIndex + i] = reinterpret_cast<size_t>(malloc(measureMTStack_DataTypeSize));
		//measureMTStack_DefaultAllocatorPointers[startIndex + i] = reinterpret_cast<size_t>(new int());
	}

	/* Thread is done, increase variable */
	++measureMTStack_FinishedDefaultThreads;

	/* If this is the last thread to finish, measure time */
	if (measureMTStack_FinishedDefaultThreads == measureMTStack_NumThreads)
		measureMTStack_DefaultAllocationTime = SDL_GetTicks() - measureMTStack_StartTime;

	delete _ptr;

	return 0;
}

int MeasureMTStack_ThreadDefaultFree(void* _ptr)
{
	const unsigned int id = *static_cast<unsigned int*>(_ptr);
	const unsigned int numFrees = (measureMTStack_BufferSize / measureMTStack_DataTypeSize) / measureMTStack_NumThreads;
	const unsigned int startIndex = (id * numFrees);

	while (!measureMTStack_Ready) {}

	/* Free memory */
	for (unsigned int i = 0; i < numFrees; ++i)
	{
		free(reinterpret_cast<void*>(measureMTStack_DefaultAllocatorPointers[startIndex + i]));
		//delete(reinterpret_cast<int*>(measureMTStack_DefaultAllocatorPointers[startIndex + i]));
	}

	/* Thread is done, increase variable */
	++measureMTStack_FinishedDefaultThreads;

	/* If this is the last thread to finish, measure time */
	if (measureMTStack_FinishedDefaultThreads == measureMTStack_NumThreads)
		measureMTStack_DefaultFreeTime = SDL_GetTicks() - measureMTStack_StartTime;

	delete _ptr;

	return 0;
}

void MeasureMTStack(unsigned int _numAllocations, unsigned int _dataTypeSize, unsigned int _numThreads, unsigned int _alignment)
{
	measureMTStack_BufferSize = _numAllocations * _dataTypeSize;
	measureMTStack_DataTypeSize = _dataTypeSize;
	measureMTStack_NumThreads = _numThreads;

	/* Create a memory buffer */
	measureMTStack_Buffer = 0;
	try
	{
		measureMTStack_Buffer = new Memory::StackAllocator_SingleBuffer(measureMTStack_BufferSize, _alignment);
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	/* Initialize two arrays of pointers, used for storing allocations */
	measureMTStack_StackPointers = new size_t[_numAllocations]; // This array is actually not used for anything. Exists to even out the overhead performance
	measureMTStack_DefaultAllocatorPointers = new size_t[_numAllocations];

	/* Create thread array */
	SDL_Thread** threads = new SDL_Thread*[_numThreads];

#pragma region Stack Allocation Time
	measureMTStack_Ready = false;
	measureMTStack_FinishedStackThreads = 0;

	/* Create threads for stack allocation */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "StackThread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(MeasureMTStack_ThreadStackAlloc, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create stack thread %i\n", i);
			abort();
		}
	}

	/* Begin measure time */
	measureMTStack_StartTime = SDL_GetTicks();

	/* Tell the threads to start allocating */
	measureMTStack_Ready = true;

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
	measureMTStack_Ready = false;
	measureMTStack_FinishedDefaultThreads = 0;

	/* Create threads for default allocator allocation */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "DefaultThread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(MeasureMTStack_ThreadDefaultAlloc, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create default thread %i\n", i);
			abort();
		}
	}

	/* Begin measure time */
	measureMTStack_StartTime = SDL_GetTicks();

	/* Tell the threads to start allocating */
	measureMTStack_Ready = true;

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

#pragma region Stack Free Time
	/* Measure stack free time */
	measureMTStack_StartTime = SDL_GetTicks();
	measureMTStack_Buffer->FreeTo(0);
	measureMTStack_StackFreeTime = SDL_GetTicks() - measureMTStack_StartTime;
#pragma endregion

#pragma region Default Allocator Free Time
	measureMTStack_Ready = false;
	measureMTStack_FinishedDefaultThreads = 0;

	/* Create threads for default allocator allocation */
	for (unsigned int i = 0; i < _numThreads; ++i)
	{
		std::string name = "DefaultThread " + i;
		unsigned int* id = new unsigned int(i);
		threads[i] = SDL_CreateThread(MeasureMTStack_ThreadDefaultFree, name.c_str(), id);

		/* Check for failure when creating thread */
		if (threads[i] == NULL)
		{
			printf("Couldn't create default thread %i\n", i);
			abort();
		}
	}

	/* Begin measure time */
	measureMTStack_StartTime = SDL_GetTicks();

	/* Tell the threads to start freeing memory */
	measureMTStack_Ready = true;

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

	printf("Results from MeasureMTStack():\n");
	printf("Number of Threads: %i\n", _numThreads);
	printf("Buffer Size: %i\n", measureMTStack_BufferSize);
	printf("Data Type Size: %i\n", measureMTStack_DataTypeSize);
	printf("Number of Allocated Data Types per Thread: %i\n\n", (_numAllocations / _numThreads));
	printf("Statistics:\n");
	printf("Multi-Threaded Stack Allocation Time: %ims\n", measureMTStack_StackAllocationTime);
	printf("Multi-Threaded Default Allocator Allocation Time: %ims\n", measureMTStack_DefaultAllocationTime);
	printf("Sequential Stack Free Time: %ims (Multi-threaded version would be unnecessary)\n", measureMTStack_StackFreeTime);
	printf("Multi-Threaded Default Allocator Free Time: %ims\n", measureMTStack_DefaultFreeTime);
	printf("-------------------------------\n\n");

	/* Free memory */
	delete[] threads;
	delete[] measureMTStack_StackPointers;
	delete[] measureMTStack_DefaultAllocatorPointers;
	delete measureMTStack_Buffer;
}
#pragma endregion

#pragma endregion

#pragma region Pool
void VerifyMTPool()
{

}

void TestSQPool()
{

}

void TestMTPool()
{

}
#pragma endregion

#endif
