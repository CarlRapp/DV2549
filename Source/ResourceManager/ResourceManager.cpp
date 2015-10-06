#include "ResourceManager.h"
#include <assert.h>

ResourceManager::ResourceManager()
	: m_totalMemorySize(0), m_currentAllocatedMemory(0), m_loadedChunks(0), m_ticks(0)
{

}

ResourceManager::~ResourceManager()
{


}

ResourceManager& ResourceManager::GetInstance()
{
	static ResourceManager* rmInstance = new ResourceManager();

	return *rmInstance;
}

bool ResourceManager::InitResourceManager(size_t _totalMemorySize)
{
	m_currentAllocatedMemory = _totalMemorySize;

	return false;
}

bool ResourceManager::LoadAsset()
{
	//	Check if the asset already exists?

	//	Calculate size for wanted asset
	size_t	assetSize = 0;

	//	Check if the requested asset fits in our memory 
	//	(not very accurate check if fragmentation exists)
	if (m_currentAllocatedMemory + assetSize > m_totalMemorySize)
	{
		//	No more memory, either crash or check if
		//	we can remove old assets

		return false;
	}

	return true;
}

bool ResourceManager::UnloadAsset()
{
	//	Find the asset

	//	Set that memory as "free" but let the data remain
	//	or actually delete the data, dunno what we wanna do?


	return true;
}

void ResourceManager::CreateChunkPool(unsigned int _nChunks)
{
	//	If the current loaded chunks
	if(m_loadedChunks)
	{
		m_currentAllocatedMemory	-=	m_loadedChunksN*sizeof(LoadedChunk);
		delete m_loadedChunks;
	}
		

	//m_loadedChunks = new PoolAllocator(_nChunks, sizeof(LoadedChunk));
	//for (int n = 0; n < _nChunks; ++n)
	//	m_loadedChunks->Allocate();

	m_loadedChunks = new LoadedChunk[_nChunks];

	std::vector<Graphics::GraphicsWrapper::TerrainPatch*> newPatchPointers;

	m_currentAllocatedMemory += _nChunks*sizeof(LoadedChunk);
	for (int n = 0; n < _nChunks; ++n)
	{
		newPatchPointers.push_back(&m_loadedChunks[n].GraphicsPatch);
		m_loadedChunks[n].Popularity = n;
	}
	m_graphicsWrapper->ReloadTerrainPatches(newPatchPointers);
		
	m_loadedChunksN = _nChunks;
	
	//	No more memory...
	assert(m_currentAllocatedMemory > m_totalMemorySize);
}

void ResourceManager::LoadChunk(int tileX, int tileZ)
{
	//Graphics::GraphicsWrapper::TerrainPatch* tileMemLoc = 0;// (Graphics::GraphicsWrapper::TerrainPatch*)

	LoadedChunk*	chunkToOverwrite = &m_loadedChunks[GetLeastPopularChunkIndex()];
	if (chunkToOverwrite)
	{
		m_graphicsWrapper->DeleteSingleTexturePatch(chunkToOverwrite->X, chunkToOverwrite->Z);
		m_graphicsWrapper->LoadSingleTexturePatch(tileX, tileZ, &chunkToOverwrite->GraphicsPatch);
		chunkToOverwrite->X = tileX;
		chunkToOverwrite->Z = tileZ;
		chunkToOverwrite->Popularity = SDL_GetTicks();
	}
	else
		std::printf("MAX NUMBER OF LOADED CHUNKS!\n");
}

void ResourceManager::Update(float _dt)
{
	for (int n = 0; n < m_loadedChunksN; ++n)
		m_loadedChunks[n].Popularity += 1;
		

	m_ticks++;
}

int ResourceManager::GetLeastPopularChunkIndex()
{
	int lowestPopularity = INT_MAX;
	int returnIndex = 0;
	for (int n = 0; n < m_loadedChunksN; ++n)
	{
		LoadedChunk tChunk = m_loadedChunks[n];

		if (tChunk.Popularity < lowestPopularity)
		{
			lowestPopularity = tChunk.Popularity;
			returnIndex = n;
		}
	}


	return returnIndex;
}
