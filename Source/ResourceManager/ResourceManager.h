#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#endif
#else
#endif

#include <SDL/SDL.h>
#include <string>

#include "Memory/PoolAllocator/PoolAllocator.h"
#include "Memory/StackAllocator/StackAllocator_SingleBuffer.h"
#include "Graphics/GraphicsWrapper.h"

class DECLSPEC ResourceManager
{
public:
	struct LoadedChunk
	{
		Graphics::GraphicsWrapper::TerrainPatch GraphicsPatch;
		size_t Popularity;
		int X;
		int Z;
	};

public:
	~ResourceManager();
	static ResourceManager& GetInstance();

	bool InitResourceManager(size_t _totalMemorySize);
	void SetGraphicsWrapper(Graphics::GraphicsWrapper* instance);

	bool LoadAsset();
	bool UnloadAsset();

	void CreateChunkPool(unsigned int _nChunks);
	//void LoadChunk(int tileX, int tileY);


	void LoadChunks_Thread();
	void LoadChunk_Threaded(int x, int y);
	std::vector<LoadedChunk> GetLoadedTextures();


	void Update(float _dt);
	

private:
	ResourceManager();



private:
	size_t	m_currentAllocatedMemory;
	size_t	m_totalMemorySize;

	Graphics::GraphicsWrapper*	m_graphicsWrapper;


	size_t			m_lastCheckedChunk;
	size_t			m_loadedChunksN;
	LoadedChunk*	m_loadedChunks;
	
	int	GetLeastPopularChunkIndex();


	SDL_Thread* m_thread;
	SDL_mutex* m_mutex;

	bool stop;
	std::vector<LoadedChunk> m_preloadedChunks;
	std::queue<SDL_Point> m_chunksToPreload;

	size_t m_ticks;

	HDC m_hDC;
	HGLRC m_resourceContext;
};


#endif