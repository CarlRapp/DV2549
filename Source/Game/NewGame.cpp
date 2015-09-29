#include <SDL/SDL.h>
#include <GLEW/glew.h>
#include "Input/InputWrapper.h"
#include "Graphics/GraphicsWrapper.h"
#include "Memory/MemoryWrapper.h"
#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"
#include "Memory/StackAllocator/StackAllocator_DoubleEnded.h"
#include "Memory/TestScenarios.h"
#include "Memory/PoolTest.h"
#include "GameManager.h"
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

int main(int argc, char** argv)
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
	GameManager	gameManager = GameManager::GetInstance();
	Graphics::GraphicsWrapper graphics = Graphics::GraphicsWrapper::GetInstance();

	//SETTINGS
	int width = 1024;
	int height = 840;
	int centerX = width/2;
	int centerY = height/2;
	const float cameraSpeed = 9.0f;
	const float mouseSensitivity = 3.0f;
	bool lockMouse = true;

	//INIT GFX
	graphics.InitializeSDL(width, height);
	graphics.InitializeGLEW();
	graphics.InitializeShaders();
	graphics.LoadTerrainPatch();
	gameManager.SetGraphicsWrapper(&graphics);

	graphics.GetCamera()->SetPosition(glm::vec3(0, 2, 0));
	graphics.GetCamera()->SetForward(glm::vec3(0, 0, -1));

	

	//INIT INPUT
	Input::InputWrapper input = Input::InputWrapper::GetInstance();
	input.GetMouse()->SetCenter(centerX, centerY);

	//DELTA TIME
	Uint32 beginFrame = 0, endFrame = 0;
	double t = 0.0;
	double dt = 1 / 60.0;

	bool quit = false;
	while (!quit)
	{
		beginFrame = SDL_GetTicks();
		double frameTime = (beginFrame - endFrame)*0.001;
		endFrame = beginFrame;

		while (frameTime > 0.0)
		{
			float deltaTime = (float)std::min(frameTime, dt);
			frameTime -= deltaTime;
			t += deltaTime;

			input.Update();
			SDL_Event e;
			SDL_PollEvent(&e);
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
				input.PollEvent(e);
				break;
			case SDL_QUIT:
				quit = true;
				break;
			}

			

			//In game commands
			//Move to some nice place
			{
				//QUIT
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_ESCAPE))
					quit = true;

				//WASD MOVEMENT
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_W))
					graphics.MoveCameraForward(-cameraSpeed*deltaTime);
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_S))
					graphics.MoveCameraForward(cameraSpeed*deltaTime);
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_A))
					graphics.MoveCameraStrafe(-cameraSpeed*deltaTime);
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_D))
					graphics.MoveCameraStrafe(cameraSpeed*deltaTime);

				//MOUSELOOK
				int dx = input.GetMouse()->GetdX();
				int dy = input.GetMouse()->GetdY();
				if (abs(dx) > 0.0f)
					graphics.LookCameraX(-dx*deltaTime*mouseSensitivity);
				if (abs(dy) > 0.0f)
					graphics.LookCameraY(-dy*deltaTime*mouseSensitivity);

				//LOCK MOUSE IN CENTER
				if(lockMouse)
					SDL_WarpMouseInWindow(graphics.GetWindow(), centerX, centerY);
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_F))
					lockMouse = !lockMouse;
			}

			gameManager.Update(deltaTime);
		}

		graphics.RenderTerrain();
	}
	
	return 0;
}