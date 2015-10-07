#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#endif
#else
#endif
#include <windows.h>
#include "GLShaderHandler.h"
#include "GLCamera.h"
#include <SDL/SDL.h>

#include <SDL/SDL_syswm.h>


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
			GLuint			TextureNormal;
			GLuint			VAO;

			std::vector<RenderInstance*> Instances;
		};



		

		struct Level
		{
			int TileSize = 2;
			int PatchSize = 16;

			unsigned int Width		= 10000;
			unsigned int Height		= 5000;
			unsigned int ChunkSize = 256;
 			unsigned int X			= 0;
 			unsigned int Y			= 0;

			float* TerrainVertices;
			float* TerrainTex;
			float* TerrainNormals;

			unsigned int Vertices;
			unsigned int TexCoords;
			unsigned int Normals;
		};
	public:
		struct TerrainPatch
		{
			glm::mat4		ModelMatrix;
			GLuint			TextureHeight;
			GLuint			TextureNormal;
			GLuint			TextureDiffuse;
		};



		bool SDLStarted() const { return m_SDLStarted; }

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

		void ReloadTerrainPatches(std::vector<TerrainPatch*> newPatches);
		void LoadSingleTexturePatch(int tileX, int tileY, TerrainPatch* memLocation);
		void DeleteSingleTexturePatch(int tileX, int tileY);
		void DeleteSingleTexturePatch(TerrainPatch* memLocation);
		Level* GetLevel() { return &m_level; }

		GLuint LoadTextureRAW(const char * _filename, unsigned int _width, unsigned int _height, short _colorSlots);
		GLuint LoadTexturePatch(const char * _filename, unsigned int _x, unsigned int _y, short _colorSlots);
		void ConvertToPAK(const char * _filename, GLint _width, GLint _height, short colorSlots);
		SDL_Window* GetWindow() { return m_window; };

		GLCamera*	GetCamera() { return m_camera; }

		unsigned int AddString(std::string* _text, glm::vec3 _color, float _scale, float _x, float _y);

		void FogToggle() { m_fog = !m_fog; }

		HDC GetHDC();

		HGLRC GetHGLRC() { return m_renderContext; }

	private:
		GraphicsWrapper();

		unsigned int m_width = 0;
		unsigned int m_height = 0;

		SDL_Window* m_window;
		SDL_GLContext m_context;

		GLuint m_shaderVSStandard;

		ShaderHandler* m_terrainShader;
		

		GLCamera* m_camera;

		std::vector<RenderItem*> m_renderItems;
		std::vector<TerrainPatch*> m_terrainPatches;
		
		GLuint m_terrainVAO;
		GLuint m_terrainVBO[3];

		Level m_level;

		std::vector<std::vector<TerrainPatch*>> m_mapStatus;

		bool m_fog = true;

		HGLRC m_renderContext;

		bool m_SDLStarted = false;

		HDC m_hDC;
	};
}

#endif