#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#endif
#else
#endif

#include <SDL/SDL.h>
#include <vector>
#include <string>

namespace Graphics
{
	class DECLSPEC GraphicsWrapper
	{
	public:
		~GraphicsWrapper();
		static GraphicsWrapper& GetInstance();

		void Render();

	private:
		GraphicsWrapper();
		static	GraphicsWrapper*	m_instance;

		SDL_Window*	m_graphicsWindow;
		SDL_GLContext m_glContext;
	};
}

#endif