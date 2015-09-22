#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#endif
#else
#endif
#include "GLShaderHandler.h"
#include "GLCamera.h"
#include <SDL/SDL.h>

namespace Graphics
{
	class DECLSPEC GraphicsWrapper
	{
	public:
		~GraphicsWrapper();
		static GraphicsWrapper& GetInstance();

		void Render();

		void CameraToShader();

		void InitializeSDL(unsigned int _width, unsigned int _height);
		void InitializeGLEW();
		void LoadModel();
		void InitializeShaders();

		void MoveCameraForward(float _val);
		void MoveCameraStrafe(float _val);
		void LookCameraX(float _val);
		void LookCameraY(float _val);

		SDL_Window* GetWindow() { return m_window; };

	private:
		GraphicsWrapper();
		static	GraphicsWrapper*	m_instance;

		unsigned int m_width = 0;
		unsigned int m_height = 0;

		SDL_Window* m_window;
		SDL_GLContext m_context;

		GLuint m_shaderVSStandard;

		ShaderHandler* m_shaderSTD;

		GLCamera* m_camera;
	};
}

#endif