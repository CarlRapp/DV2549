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

#define FORMAT_RAW 0
#define FORMAT_PAK 1

namespace Graphics
{



	class DECLSPEC GraphicsWrapper
	{
		struct RenderInstance
		{
			glm::mat4 ModelMatrix;
		};

		struct RenderItem
		{
			GLuint			MatrixBuffer;
			int				RenderParam = GL_TRIANGLES;
			unsigned int	Vertices;
			GLuint			TextureDiffuse;
			GLuint			VAO;

			std::vector<RenderInstance*> Instances;
		};

		struct TerrainPatch
		{
			glm::mat4		ModelMatrix;
			GLuint			TextureDiffuse;
		};

		struct Level
		{
			unsigned int Width = 12000;
			unsigned int Height = 6000;
			unsigned int ChunkSize = 512;
 			unsigned int X			= 23;
 			unsigned int Y			= 11;
		};



	public:
		~GraphicsWrapper();
		static GraphicsWrapper& GetInstance();

		void Render();
		void RenderTerrain();

		void InitializeSDL(unsigned int _width, unsigned int _height);
		void InitializeGLEW();
		void LoadTerrainPatch();
		void LoadModel();
		void InitializeShaders();

		void MoveCameraForward(float _val);
		void MoveCameraStrafe(float _val);
		void LookCameraX(float _val);
		void LookCameraY(float _val);

		GLuint LoadTexturePatch(const char * _filename, unsigned int _x, unsigned int _y, short _colorSlots, GLint _fileFormat);
		void ConvertToPAK(const char * _filename, GLint _width, GLint _height, short colorSlots);
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

		std::vector<RenderItem*> m_renderItems;
		std::vector<TerrainPatch*> m_terrainPatches;
		
		GLuint m_terrainVAO;
		GLuint m_terrainVBO[3];

		Level m_level;
	};
}

#endif