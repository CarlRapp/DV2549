#include <SDL/SDL.h>
#include <GLEW/glew.h>
#include "Input/InputWrapper.h"
#include "Graphics/GraphicsWrapper.h"
#include "Memory/MemoryWrapper.h"
#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"
#include "Memory/TestScenarios.h"

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include <new>
#include <iostream>
using namespace std;
class A
{
public:
	int x;

	A(int px)
	{
		x = px;
	}

	~A()
	{}
};

class B
{
public:
	int x; int y;

	B(int px, int py)
	{
		x = px; y = py;
	}

	~B()
	{}
};

class C
{
public:
	int x; float y; int z;

	C(int px, float py, int pz)
	{
		x = px; y = py; z = pz;
	}

	~C()
	{}
};


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


int main(int argc, char** argv)
{
	Input::InputWrapper inputWrapper = Input::InputWrapper::GetInstance();
	Graphics::GraphicsWrapper graphicsWrapper = Graphics::GraphicsWrapper::GetInstance();
	Memory::MemoryWrapper memoryWrapper = Memory::MemoryWrapper::GetInstance();

	VerifyMTStack(4096, 4, 4);
	MeasureSQStack(1048576, 4);
	MeasureMTStack(1048576, 4, 4);

	Memory::StackAllocator_SingleBuffer* singleBuffer = 0;
	try
	{
		singleBuffer = new Memory::StackAllocator_SingleBuffer(1024);
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	int* startAddress = (int*)singleBuffer->Reserve(0);

	for (int n = 0; n < 256; ++n)
		startAddress[n] = 0;

	singleBuffer->FreeTo(0);
	try
	{
		void* address = 0;
		while (true)
		{
			inputWrapper.Update();
			SDL_Event e;
			#pragma region	PollEvent
			while (SDL_PollEvent(&e))
			{
				switch (e.type)
				{
				case SDL_WINDOWEVENT:
					break;
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				case SDL_FINGERMOTION:
				case SDL_FINGERDOWN:
				case SDL_FINGERUP:
				case SDL_TEXTINPUT:
				case SDL_JOYAXISMOTION:
				case SDL_JOYBALLMOTION:
				case SDL_JOYHATMOTION:
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEWHEEL:
				case SDL_MULTIGESTURE:
					inputWrapper.PollEvent(e);
					break;
				}
			}
	#pragma endregion

			
			
			std::vector<SDL_Thread*> threads;
			std::printf("\n\nBefore:\n");
			for (int n = 0; n < 10; ++n)
				std::printf("%i | ", startAddress[n]);
			
			for (int n = 0; n < 10; ++n)
			{
				SDL_Thread* tThread = SDL_CreateThread(TestThread, "Thread", singleBuffer);
				threads.push_back(tThread);
			}

			std::printf("\n\nAfter:\n");
			for (int n = 0; n < 10; ++n)
			{
				int tempResult;
				SDL_WaitThread(threads[n], &tempResult);
				std::printf("%i | ", startAddress[n]);
			}


			//address = singleBuffer->Reserve(sizeof(A));

			//A* p_A = singleBuffer->Reserve<A>(P1, P2, P3, P4..... P5)
			/* *p_A = new (address) A(1);
			cout << endl << endl;
			cout << p_A->x << ".";

			address = singleBuffer->Reserve(sizeof(B));
			B *p_B = new (address) B(1, 2);
			cout << endl << endl;
			cout << p_B->x << ", ";
			cout << p_B->y << ".";

			address = singleBuffer->Reserve(sizeof(C));
			C *p_C = new (address) C(1, 2.5, 3);
			cout << endl << endl;
			cout << p_C->x << ", ";
			cout << p_C->y << ", ";
			cout << p_C->z << ".";
			*/
			singleBuffer->FreeTo(0);
			for (int n = 0; n < 256; ++n)
				startAddress[n] = 0;

			if (inputWrapper.GetKeyboard()->GetKeyState(SDL_SCANCODE_ESCAPE) == Input::InputState::PRESSED)
				break;


			graphicsWrapper.Render();
		}
	}
	catch (std::string e)
	{
		std::printf("Following error: %s\n", e.c_str());
	}

	
#ifdef WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
#endif
	return 0;
}