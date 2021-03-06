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
		size_t Popularity= 0;
		int X= INT32_MAX;
		int Z= INT32_MAX;
	};

public:
	~ResourceManager();
	static ResourceManager& GetInstance();

	bool InitResourceManager(size_t _totalMemorySize);
	void SetGraphicsWrapper(Graphics::GraphicsWrapper* instance);

	void CreateChunkPool(unsigned int _nChunks);
	//void LoadChunk(int tileX, int tileY);


	void LoadChunks_Thread();
	void LoadChunk_Threaded(int x, int y);
	std::vector<LoadedChunk> GetLoadedTextures();


	void Update(float _dt);
	
	void DumpCurrentMemory();

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

	HDC m_hDC;
	HGLRC m_resourceContext;
};


#endif