#include "ResourceManager.h"
#include <assert.h>

ResourceManager::ResourceManager()
	: m_totalMemorySize(0), m_currentAllocatedMemory(0), m_loadedChunks(0)
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
		m_loadedChunks[n].LastSeen = _nChunks;
	}
	m_graphicsWrapper->ReloadTerrainPatches(newPatchPointers);
		
	m_loadedChunksN = _nChunks;
	
	//	No more memory...
	assert(m_currentAllocatedMemory > m_totalMemorySize);
}

void ResourceManager::LoadChunk(int tileX, int tileZ)
{
	//Graphics::GraphicsWrapper::TerrainPatch* tileMemLoc = 0;// (Graphics::GraphicsWrapper::TerrainPatch*)

	LoadedChunk* lastSeenChunk = &m_loadedChunks[0];
	int lastSeen = m_loadedChunks[0].LastSeen;
	//	Go through and update LastSeen
	for (int n = 0; n < m_loadedChunksN; ++n)
	{
		LoadedChunk* tChunk = &m_loadedChunks[n];

		//	Update LastSeen
		if (tileX == tChunk->X && tileZ == tChunk->Z)
			tChunk->LastSeen++;

		if (tChunk->LastSeen < lastSeen)
		{
			lastSeenChunk = &m_loadedChunks[n];
			lastSeen = tChunk->LastSeen;
			tChunk->LastSeen++;
		}
	}

	if (lastSeenChunk)
	{
		m_graphicsWrapper->LoadSingleTexturePatch(tileX, tileZ, &lastSeenChunk->GraphicsPatch);
		lastSeenChunk->X = tileX;
		lastSeenChunk->Z = tileZ;
		lastSeenChunk->LastSeen = 0;
	}
	else
		std::printf("MAX NUMBER OF LOADED CHUNKS!\n");
}
