#include "windows.h"
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
#include "Timer/Timer.h"

#include "psapi.h"

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

GameManager*	gameManager;
Graphics::GraphicsWrapper* graphics;
ResourceManager* resource;
//Memory::MemoryWrapper* memory;

float deltaTime = 0;
bool quit = false;

int main(int argc, char** argv)
{
	printf("	M Thread: STARTUP\n");

	//SDL_LockMutex(Global::gMutex);
	//derpTests();
	gameManager = &GameManager::GetInstance();
	graphics = &Graphics::GraphicsWrapper::GetInstance();
	
	unsigned int chunkSize = graphics->GetLevel()->ChunkSize;

	Memory::MemoryWrapper::GetInstance()->CreateGlobalStack(
		chunkSize*chunkSize * 3 +	//normalmap
		chunkSize*chunkSize * 3 +	//diffusemap
		chunkSize*chunkSize * 1,	//heightmap
		8);

	graphics->gMutex = SDL_CreateMutex();
	graphics->gCond = SDL_CreateCond();
	gameManager->SetGraphicsWrapper(graphics);
	

	//SETTINGS
	int width = 1280;
	int height = 720;
	int centerX = width/2;
	int centerY = height/2;
	const float cameraSpeed = 100.0f;
	int cameraMaxY = 40;
	const float mouseSensitivity = 9.0f;
	bool lockMouse = true;

	//INIT GFX
	
	graphics->InitializeSDL(width, height);

	wglMakeCurrent(NULL, NULL);
	resource = &ResourceManager::GetInstance();
	//resource->SetGraphicsWrapper(graphics);


	//SDL_CondSignal(graphics->gCond);
	printf("	M Thread: SDL OK\n");

	printf("	M Thread: WAIT SHARE LIST...\n");
	SDL_CondWait(graphics->gCond, graphics->gMutex);
	printf("	M Thread: SHARE LIST OK\n");

	graphics->InitializeGLEW();
	graphics->InitializeShaders();
	graphics->LoadTerrainPatch();
	

	graphics->GetCamera()->SetPosition(glm::vec3(graphics->GetLevel()->PatchSize*0.5f, cameraMaxY, graphics->GetLevel()->PatchSize*0.5f));
	graphics->GetCamera()->SetForward(glm::vec3(0, -1, 0));
	

	//std::string FPS;
	std::string fpsString = "fps";
	std::string infoString = "FOG (F)\nMOUSE LOCK (G)";
	std::string renderDistance = "RENDER DISTANCE: 1";

	graphics->AddString(&fpsString, glm::vec3(0, 1, 0), 2, 0, 0);
	graphics->AddString(&renderDistance, glm::vec3(0, 1, 1), 2, 0, -250);
	graphics->AddString(&infoString, glm::vec3(1, 1, 0), 2, 0, -150);
	
	resource->SetGraphicsWrapper(graphics);
	gameManager->RequestRenderDistance(3);
	//INIT INPUT
	Input::InputWrapper input = Input::InputWrapper::GetInstance();
	input.GetMouse()->SetCenter(centerX, centerY);

	//DELTA TIME
	Uint32 beginFrame = 0, endFrame = 0;
	double t = 0.0;
	double dt = 1 / 60.0;
	Utility::Timer fTimer = Utility::Timer();
	

	//MEMORY USAGE
	PROCESS_MEMORY_COUNTERS memCounter;

	gameManager->Update(deltaTime);

	printf("	M Thread: BIG INIT OK\n");
	SDL_CondSignal(graphics->gCond);
	//SDL_UnlockMutex(Global::gMutex);

	printf("	M Thread: RUNNING\n");

	while (!quit)
	{
		fTimer.Tick();
		beginFrame = SDL_GetTicks();
		double frameTime = fTimer.GetDeltaTime(); // (beginFrame - endFrame)*0.001;
		endFrame = beginFrame;

		GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));

		fpsString = "DTT: " + std::to_string(frameTime) + "MS";
		fpsString += "\nFPS: " + std::to_string( int(1.0 / frameTime));
		fpsString += "\nMEM: " + std::to_string(memCounter.PagefileUsage/(1024*1024)) + "MB";

		while (frameTime > 0.0)
		{
			deltaTime = (float)(std::min(frameTime, dt));
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
					graphics->MoveCameraForward(-cameraSpeed*deltaTime);
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_S))
					graphics->MoveCameraForward(cameraSpeed*deltaTime);
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_A))
					graphics->MoveCameraStrafe(-cameraSpeed*deltaTime);
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_D))
					graphics->MoveCameraStrafe(cameraSpeed*deltaTime);

				//CAMERA MAX HEIGHT
				glm::vec3 camPos = graphics->GetCamera()->GetPosition();
				if (camPos.y < cameraMaxY)
				{
					camPos.y = (float)cameraMaxY;
					graphics->GetCamera()->SetPosition(camPos);
				}

				//MOUSELOOK
				if(lockMouse)
				{
					int dx = input.GetMouse()->GetdX();
					int dy = input.GetMouse()->GetdY();
					if (abs(dx) > 0.0f)
						graphics->LookCameraX(-dx*deltaTime*mouseSensitivity);
					if (abs(dy) > 0.0f)
						graphics->LookCameraY(-dy*deltaTime*mouseSensitivity);

				}

				//LOCK MOUSE IN CENTER
				if(lockMouse)
					SDL_WarpMouseInWindow(graphics->GetWindow(), centerX, centerY);
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_G) == Input::PRESSED)
					lockMouse = !lockMouse;

				//TOGGLE FOG
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_F) == Input::PRESSED)
					graphics->FogToggle();

				//CHANGE RENDER DISTANCE
				if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_RIGHT) == Input::PRESSED)
				{
					int currentDistance = gameManager->GetRenderDistance();
					gameManager->RequestRenderDistance(currentDistance + 1);

					renderDistance = "RENDER DISTANCE ";
					renderDistance.append(std::to_string(gameManager->GetRenderDistance()));

				}
				else if (input.GetKeyboard()->GetKeyState(SDL_SCANCODE_LEFT) == Input::PRESSED)
				{
					int currentDistance = gameManager->GetRenderDistance();
					gameManager->RequestRenderDistance(currentDistance - 1);

					renderDistance = "RENDER DISTANCE ";
					renderDistance.append(std::to_string(gameManager->GetRenderDistance()));
				}
					
					
			}

			
		}
		gameManager->Update(deltaTime);
		resource->Update(deltaTime);
		graphics->RenderTerrain();
	}


	return 0;
}

