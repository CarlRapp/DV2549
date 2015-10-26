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


/*
#include "ResourceManager\Compression\CompressionHandler_lz4.h" // Temp. include until integration with ResourceManager.
#include "ResourceManager\Compression\CompressionHandler_zlib.h" // Temp. include until integration with ResourceManager.*/
#include "PakLib/PackageReaderWriter.h"

namespace Graphics
{
	struct TextureRAM
	{
		short ColorSlots;
		GLubyte* Data;
		int Width, Height;
	};

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
			int TileSize = 64;
			int PatchSize = 256;

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
			glm::mat4		ModelMatrix = glm::mat4(1.0f);
			GLuint			TextureHeight = 0;
			GLuint			TextureNormal = 0;
			GLuint			TextureDiffuse = 0;
			bool			IsActive = false;
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
		void LoadSingleTexturePatch(int tileX, int tileY, TerrainPatch* memLocation, HDC* hdc, HGLRC* hglrc);
		//void DeleteSingleTexturePatch(int tileX, int tileY);
		void DeleteSingleTexturePatch(TerrainPatch* memLocation, HDC* _hdc, HGLRC* _hglrc);
		void DeleteSingleTexturePatch(TerrainPatch* memLocation);
		Level* GetLevel() { return &m_level; }

		//GLuint LoadTextureRAW(const char * _filename, unsigned int _width, unsigned int _height, short _colorSlots);
		Graphics::TextureRAM PushTextureToRAM(const char * _filename, unsigned int _x, unsigned int _y, short _colorSlots);


		GLuint PushTextureToGL(TextureRAM tex);

		void ConvertToPAK(const char * _filename, GLint _width, GLint _height, short colorSlots);
		SDL_Window* GetWindow() { return m_window; };
			
		//Compression::ICompressionHandler *compressionHandler; // Temp. variable until integration with ResourceManager.

		GLCamera*	GetCamera() { return m_camera; }

		unsigned int AddString(std::string* _text, glm::vec3 _color, float _scale, float _x, float _y);

		void FogToggle() { m_fog = !m_fog; }

		HDC GetHDC();

		HGLRC GetHGLRC() { return m_renderContext; }

		SDL_mutex* gMutex;
		SDL_cond* gCond;

	private:
		GraphicsWrapper();

		unsigned int m_width = 0;
		unsigned int m_height = 0;

		SDL_Window* m_window;
		SDL_GLContext m_context;

		GLuint m_shaderVSStandard;

		ShaderHandler* m_terrainShader;
		

		GLCamera* m_camera;

		std::vector<TerrainPatch*> m_terrainPatches;
		
		GLuint m_terrainVAO;
		GLuint m_terrainVBO[3];

		Level m_level;

		std::vector<std::vector<TerrainPatch*>> m_mapStatus;

		bool m_fog = true;

		HGLRC m_renderContext;

		bool m_SDLStarted = false;

		HDC m_hDC;

		//SDL_mutex* m_mutex;

		std::string debugString = "";

		PackageReaderWriter *m_pakRW;
	};
}

#endif