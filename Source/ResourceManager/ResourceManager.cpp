#include "ResourceManager.h"

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

void ResourceManager::CreateChunkPool(unsigned int _nChunks, unsigned int _chunkSize)
{
	//	If the current loaded chunks
	if(m_loadedChunks)
		delete m_loadedChunks;

	m_loadedChunks = new PoolAllocator(_nChunks, _chunkSize);
}
