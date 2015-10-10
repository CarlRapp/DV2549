#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#include "Memory/StackAllocator/StackAllocator_DoubleEnded.h"
#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"
#include "Memory/MemoryWrapper.h"

static int TestThread(void* dataPtr)
{
	int tCounter = 0;
	SDL_threadID tId = SDL_ThreadID();
	while (true)
	{
		SDL_Delay(std::rand() % 3);
		Memory::StackAllocator_SingleBuffer* _stackBuffer = (Memory::StackAllocator_SingleBuffer*)dataPtr;
		int* threadData = (int*)_stackBuffer->Reserve(sizeof(int));

		if (*threadData != 0)
		{
			int value = *threadData;

			int a = 2;
		}


		*threadData = tId;
		++tCounter;

		if (tCounter == 20)
			break;
	}
	std::printf("%i | ", tId);

	return 0;
}

static int TestDoubleEnded(void* ptr)
{
	SDL_threadID tId = SDL_ThreadID();
	Memory::StackAllocator_SingleBuffer* _stackBuffer = (Memory::StackAllocator_SingleBuffer*)ptr;
	SDL_Delay(std::rand() % 3);
	_stackBuffer->Push<int>(tId);


	return 0;
}

void TempTests()
{
	int stackSize = 1024;
	int alignment = 4;
	//Memory::StackAllocator_DoubleEnded* stack = new Memory::StackAllocator_DoubleEnded(stackSize, alignment);
	Memory::MemoryWrapper::GetInstance()->CreateGlobalStack(stackSize, alignment);
	Memory::MemoryWrapper::GetInstance()->CreateStack(stackSize, alignment);
	Memory::IStackAllocator* stack = Memory::MemoryWrapper::GetInstance()->GetGlobalStack();// (stackSize, alignment);
	int* start = (int*)stack->Reserve(0);
	for (int n = 0; n < (int)stackSize / alignment; ++n)
		start[n] = 0;


	printf("Before:\n");
	for (int n = 0; n < (int)stackSize / alignment; ++n)
		if ((n + 1) % 10 == 0)
			printf("\n");
	printf("\n");

	int nThreads = 100;
	std::vector<SDL_Thread*> threads;
	for (int n = 0; n < nThreads; ++n)
	{
		threads.push_back(SDL_CreateThread(TestDoubleEnded, "THREAD", stack));
	}

	for (int n = 0; n < nThreads; ++n)
	{
		int a;
		SDL_WaitThread(threads[n], &a);
	}

	printf("After:\n");
	for (int n = 0; n < (int)stackSize / alignment; ++n)
	{
		printf("%i ", start[n]);

		if ((n + 1) % 10 == 0)
			printf("\n");
	}
	printf("\n");

	system("pause");
}

void derpTests()
{
	//TEST STUFF
	{
		//TempTests();

		/* Stack Alignment Test */
		//MeasureSQStack(4194304, 4, 4);
		//MeasureSQStack(4194304, 4, 8);
		//MeasureSQStack(4194304, 4, 16);
		//MeasureSQStack(4194304, 4, 32);
		//MeasureSQStack(4194304, 4, 64);
		//MeasureSQStack(4194304, 4, 128);
		//MeasureSQStack(4194304, 4, 256);
		//MeasureSQStack(4194304, 4, 512);
		//MeasureSQStack(4194304, 4, 1024);
		//MeasureMTStack(4194304, 4, 4, 4);
		//MeasureMTStack(4194304, 4, 4, 8);
		//MeasureMTStack(4194304, 4, 4, 16);
		//MeasureMTStack(4194304, 4, 4, 32);
		//MeasureMTStack(4194304, 4, 4, 64);
		//MeasureMTStack(4194304, 4, 4, 128);
		//MeasureMTStack(4194304, 4, 4, 256);
		//MeasureMTStack(4194304, 4, 4, 512);
		//MeasureMTStack(4194304, 4, 4, 1024);

		/* Stack Multi-Threading Test */
		//MeasureMTStack(4194304, 4, 1, 4);
		//MeasureMTStack(4194304, 4, 2, 4);
		//MeasureMTStack(4194304, 4, 3, 4);
		//MeasureMTStack(4194304, 4, 4, 4);
		//MeasureMTStack(4194304, 4, 8, 4);

		/* Pool Alignment Test (Change alignment in MemorySettings.h) */
		//MeasureSQPool(4194304, 4);
		//MeasureMTPool(4194304, 4, 4);

		/* Pool Multi-Threading Test */
		//MeasureMTPool(4194304, 4, 1);
		//MeasureMTPool(4194304, 4, 2);
		//MeasureMTPool(4194304, 4, 3);
		//MeasureMTPool(4194304, 4, 4);
		//MeasureMTPool(4194304, 4, 8);
	}
}