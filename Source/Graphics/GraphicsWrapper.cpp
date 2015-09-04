#include <GLEW/glew.h>
#include "GraphicsWrapper.h"


using namespace Graphics;
GraphicsWrapper* GraphicsWrapper::m_instance = 0;
#pragma region Constructor/Destructor and GetInstance
GraphicsWrapper::GraphicsWrapper()
{
	SDL_Init(SDL_INIT_VIDEO);

	m_graphicsWindow = SDL_CreateWindow
		(
			"GAME ENGINE",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			1280,
			720,
			SDL_WINDOW_OPENGL //| SDL_WINDOW_BORDERLESS
		);

	m_glContext = SDL_GL_CreateContext(m_graphicsWindow);

	if (glewInit() != 0)
	{

	}
	SDL_GL_SetSwapInterval(1);
}
GraphicsWrapper::~GraphicsWrapper()
{

	SDL_DestroyWindow(m_graphicsWindow);

	// Clean up
	SDL_Quit();
	m_instance = 0;
}

GraphicsWrapper& GraphicsWrapper::GetInstance()
{
	if (!m_instance)
		m_instance = new GraphicsWrapper();
	return *m_instance;
}
#pragma endregion


void GraphicsWrapper::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

	SDL_GL_SwapWindow(m_graphicsWindow);
}