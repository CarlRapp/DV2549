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
		size_t X, Z;
	};

public:
	~ResourceManager();
	static ResourceManager& GetInstance();

	bool InitResourceManager(size_t _totalMemorySize);
	void SetGraphicsWrapper(Graphics::GraphicsWrapper* instance) { m_graphicsWrapper = instance; }

	bool LoadAsset();
	bool UnloadAsset();


	void CreateChunkPool(unsigned int _nChunks);
	void LoadChunk(int tileX, int tileY);

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

	size_t m_ticks;
};


#endif