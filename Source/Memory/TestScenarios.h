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

void VerifyMTStack(unsigned int _numAllocations, unsigned int _dataTypeSize, unsigned int _numThreads)
{
	verifyMTStack_BufferSize = _numAllocations * _dataTypeSize;
	verifyMTStack_DataTypeSize = _dataTypeSize;
	verifyMTStack_NumThreads = _numThreads;

	/* Create a memory buffer */
	verifyMTStack_Buffer = 0;
	try
	{
		verifyMTStack_Buffer = new Memory::StackAllocator_SingleBuffer(verifyMTStack_BufferSize);
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
						printf("Memory has been found in more than one place (%z). Locations: (Thread: %i, Memory Index: %i) and (Thread: %i, Memory Index: %i)\n", 
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

void MeasureSQStack(unsigned int _numAllocations, unsigned int _dataTypeSize)
{
	measureSQStack_BufferSize = _numAllocations * _dataTypeSize;
	measureSQStack_DataTypeSize = _dataTypeSize;

	/* Create a memory buffer */
	measureSQStack_Buffer = 0;
	try
	{
		measureSQStack_Buffer = new Memory::StackAllocator_SingleBuffer(measureSQStack_BufferSize);
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	/* Create two arrays of pointers for allocators */
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
	}
	Uint32 defaultAllocatorAllocTime = (SDL_GetTicks() - start);

	/* Free default allocator memory */
	start = SDL_GetTicks();
	for (unsigned int i = 0; i < _numAllocations; ++i)
	{
		delete reinterpret_cast<void*>(defaultAllocatorPointers[i]);
	}
	Uint32 defaultAllocatorFreeTime = (SDL_GetTicks() - start);

	printf("Results from MeasureSQStack():\n");
	printf("Buffer Size: %i\n", measureSQStack_BufferSize);
	printf("Data Type Size: %i\n", measureSQStack_DataTypeSize);
	printf("Number of Allocated Data Types: %i\n\n", _numAllocations);
	printf("Statistics:\n");
	printf("Stack Allocation Time: %ims\n", stackAllocTime);
	printf("Default Allocator Allocation Time: %ims\n", defaultAllocatorAllocTime);
	printf("Stack Free Time: %ims\n", stackFreeTime);
	printf("Default Allocator Free Time: %ims\n", defaultAllocatorFreeTime);
	printf("-------------------------------\n\n");

	/* Free memory */
	delete[] stackPointers;
	delete[] defaultAllocatorPointers;
	delete measureSQStack_Buffer;
}
#pragma endregion

#pragma region MeasureMTStack()
void MeasureMTStack()
{
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
