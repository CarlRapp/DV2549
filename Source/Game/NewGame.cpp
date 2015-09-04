#include <SDL/SDL.h>
#include <GLEW/glew.h>
#include "Input/InputWrapper.h"
#include "Graphics/GraphicsWrapper.h"

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#endif
#endif

int main(int argc, char** argv)
{
	Input::InputWrapper inputWrapper = Input::InputWrapper::GetInstance();
	Graphics::GraphicsWrapper graphicsWrapper = Graphics::GraphicsWrapper::GetInstance();

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


	return 0;
}