#include <SDL/SDL.h>
#include <GLEW/glew.h>
#include "Input/InputWrapper.h"
#include "Graphics/GraphicsWrapper.h"
#include "Memory/MemoryWrapper.h"

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

char*	dataBlock;
int		dataBlockByteSize;
int		dataCurrentByteOffset;

void CreateBlock(int numberOfBytes)
{
	dataBlockByteSize = numberOfBytes;
	dataCurrentByteOffset = 0;
	dataBlock = (char*)malloc(numberOfBytes);
}

char* AllocateBlock(int elementSize)
{
	int tempOffset = dataCurrentByteOffset;
	dataCurrentByteOffset += elementSize;
	return dataBlock + tempOffset;
}

void ResetBlock()
{
	dataCurrentByteOffset = 0;
}

int main(int argc, char** argv)
{
	Input::InputWrapper inputWrapper = Input::InputWrapper::GetInstance();
	Graphics::GraphicsWrapper graphicsWrapper = Graphics::GraphicsWrapper::GetInstance();
	Memory::MemoryWrapper memoryWrapper = Memory::MemoryWrapper::GetInstance();

	CreateBlock(10*sizeof(int));

	std::printf("Before: \n");
	for (int n = 0; n < 10; n++)
	{
		int tempValue = dataBlock[n*sizeof(int)];
		std::printf("%i", tempValue);
	}
	std::printf("\n");


	for (int n = 0; n < 10; n++)
	{
		int* tempValue = (int*)AllocateBlock(sizeof(int));
		*tempValue = n;
	}
	std::printf("\n\nAfter: \n");
	for (int n = 0; n < 10; n++)
	{
		int tempValue = dataBlock[n*sizeof(int)];
		std::printf("%i", tempValue);
	}
	std::printf("\n");
	ResetBlock();

	while (true)
	{
		inputWrapper.Update();
		SDL_Event e;
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

		if (inputWrapper.GetKeyboard()->GetKeyState(SDL_SCANCODE_ESCAPE) == Input::InputState::PRESSED)
			break;


		graphicsWrapper.Render();
	}

	
#ifdef WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
#endif
	return 0;
}