#include <SDL/SDL.h>
#include <GLEW/glew.h>
#include "Input/InputWrapper.h"
#include "Graphics/GraphicsWrapper.h"
#include "Memory/MemoryWrapper.h"
#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"

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


int main(int argc, char** argv)
{
	Input::InputWrapper inputWrapper = Input::InputWrapper::GetInstance();
	Graphics::GraphicsWrapper graphicsWrapper = Graphics::GraphicsWrapper::GetInstance();
	Memory::MemoryWrapper memoryWrapper = Memory::MemoryWrapper::GetInstance();
	Memory::StackAllocator_SingleBuffer* singleBuffer = new Memory::StackAllocator_SingleBuffer(1024);


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





		address = singleBuffer->Reserve(sizeof(A));
		A *p_A = new (address) A(1);
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

		singleBuffer->FreeTo(0);


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