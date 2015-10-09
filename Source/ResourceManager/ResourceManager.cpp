#include "ResourceManager.h"
#include <assert.h>

static ResourceManager* m_instance = nullptr;

int StaticThreadProc(void* pParam)
{
	((ResourceManager*)pParam)->LoadChunks_Thread();
	return 0;
}


ResourceManager::ResourceManager()
	: m_totalMemorySize(0), m_currentAllocatedMemory(0), m_loadedChunks(0), m_ticks(0)
{
	stop = false;
	m_mutex = SDL_CreateMutex();
	m_thread = SDL_CreateThread(StaticThreadProc, "LoadChunks_Thread", this);
}

ResourceManager::~ResourceManager()
{


}

ResourceManager& ResourceManager::GetInstance()
{
	if (m_instance == nullptr)
		m_instance = new ResourceManager();
	return *m_instance;
}

bool ResourceManager::InitResourceManager(size_t _totalMemorySize)
{
	m_currentAllocatedMemory = _totalMemorySize;

	//compressionHandler = new Compression::CompressionHandler_zlib();

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
		for (int n = 0; n < m_loadedChunksN; ++n)
			m_graphicsWrapper->DeleteSingleTexturePatch(&m_loadedChunks[n].GraphicsPatch);

		//glFlush();
		delete m_loadedChunks;
		m_loadedChunks = 0;
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
		m_loadedChunks[n].GraphicsPatch.IsActive = false;
		m_loadedChunks[n].Popularity = SDL_GetTicks();
	}
	m_graphicsWrapper->ReloadTerrainPatches(newPatchPointers);
		
	m_loadedChunksN = _nChunks;
	
	//	No more memory...
	assert(m_currentAllocatedMemory > m_totalMemorySize);
}

void ResourceManager::LoadChunk(int tileX, int tileZ)
{
	//Graphics::GraphicsWrapper::TerrainPatch* tileMemLoc = 0;// (Graphics::GraphicsWrapper::TerrainPatch*)

	for (int n = 0; n < m_loadedChunksN; ++n)
		if (m_loadedChunks[n].X == tileX && m_loadedChunks[n].Z == tileZ)
		{
			m_loadedChunks[n].Popularity = SDL_GetTicks();
			return;
		}
			

	LoadedChunk*	chunkToOverwrite = &m_loadedChunks[GetLeastPopularChunkIndex()];
	if (chunkToOverwrite)
	{
		m_graphicsWrapper->DeleteSingleTexturePatch(&chunkToOverwrite->GraphicsPatch);
		m_graphicsWrapper->LoadSingleTexturePatch(tileX, tileZ, &chunkToOverwrite->GraphicsPatch);
		chunkToOverwrite->X = tileX;
		chunkToOverwrite->Z = tileZ;
		chunkToOverwrite->Popularity = SDL_GetTicks();
		
	}

}

void ResourceManager::Update(float _dt)
{
	SDL_LockMutex(m_mutex);
	while (!m_preloadedChunks.empty())
	{
		LoadedChunk*	chunkToOverwrite = &m_loadedChunks[GetLeastPopularChunkIndex()];
		if (chunkToOverwrite)
		{
			LoadedChunk* loadedChunk = &m_preloadedChunks.back();
			m_graphicsWrapper->DeleteSingleTexturePatch(&chunkToOverwrite->GraphicsPatch);
			memcpy(chunkToOverwrite, loadedChunk, sizeof(LoadedChunk));
			m_preloadedChunks.pop_back();
		}
		else
			break;
	}
	SDL_UnlockMutex(m_mutex);

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


void ResourceManager::LoadChunk_Threaded(int x, int y)
{
	SDL_Point point;
	point.x = x;
	point.y = y;
	SDL_LockMutex(m_mutex);
	m_chunksToPreload.push(point);
	SDL_UnlockMutex(m_mutex);
}

std::vector<ResourceManager::LoadedChunk> ResourceManager::GetLoadedTextures()
{
	SDL_LockMutex(m_mutex);
	std::vector<LoadedChunk> result = m_preloadedChunks;
	m_preloadedChunks.clear();
	SDL_UnlockMutex(m_mutex);
	return result;
}

void ResourceManager::LoadChunks_Thread()
{
	while (!m_graphicsWrapper) {}
	while (!m_graphicsWrapper->SDLStarted()) {}

	HDC hDC = m_graphicsWrapper->GetHDC();

	HGLRC resourceContext = wglCreateContext(hDC);

	wglShareLists(resourceContext, m_graphicsWrapper->GetHGLRC());

	wglMakeCurrent(hDC, resourceContext);

	LoadedChunk* chunk = new LoadedChunk();
	while (true)
	{
		SDL_LockMutex(m_mutex);
		if (!m_chunksToPreload.empty())
		{
			SDL_Point chunkToLoad = m_chunksToPreload.front();
			m_chunksToPreload.pop();
			SDL_UnlockMutex(m_mutex);

			//Ladda chunk
			//LoadChunk(chunkToLoad.x, chunkToLoad.y);
			m_graphicsWrapper->LoadSingleTexturePatch(chunkToLoad.x, chunkToLoad.y, &chunk->GraphicsPatch);
			chunk->X = chunkToLoad.x;
			chunk->Z = chunkToLoad.y;
			chunk->Popularity = SDL_GetTicks();

			SDL_LockMutex(m_mutex);
			m_preloadedChunks.push_back(*chunk);
		}

		if (stop)
		{
			SDL_UnlockMutex(m_mutex);
			delete chunk;
			return;
		}

		SDL_UnlockMutex(m_mutex);
	}
	delete chunk;
	return;
}
