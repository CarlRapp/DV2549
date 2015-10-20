#include "ResourceManager.h"
#include <assert.h>

static ResourceManager* m_instance = nullptr;

int StaticThreadProc(void* pParam)
{
	((ResourceManager*)pParam)->LoadChunks_Thread();
	return 0;
}


ResourceManager::ResourceManager()
	: m_totalMemorySize(0), m_currentAllocatedMemory(0), m_loadedChunks(0)
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
	m_totalMemorySize = _totalMemorySize;

	//compressionHandler = new Compression::CompressionHandler_zlib();

	return false;
}

void ResourceManager::SetGraphicsWrapper(Graphics::GraphicsWrapper* instance)
{
	m_graphicsWrapper = instance; 
}

void ResourceManager::CreateChunkPool(unsigned int _nChunks)
{
	//SDL_LockMutex(m_graphicsWrapper->gMutex);
	SDL_LockMutex(m_mutex);

	//	No more memory...
	if (m_currentAllocatedMemory - m_loadedChunksN*sizeof(LoadedChunk) + _nChunks*sizeof(LoadedChunk) > m_totalMemorySize)
	{
		DumpCurrentMemory();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Out of memory", "ResourceManager is using all of its memory.", NULL);
		exit(0);
	}

	//	If the current loaded chunks
	if(m_loadedChunks)
	{
		m_currentAllocatedMemory	-=	m_loadedChunksN*sizeof(LoadedChunk);
		for (int n = 0; n < m_loadedChunksN; ++n)
			m_graphicsWrapper->DeleteSingleTexturePatch(&m_loadedChunks[n].GraphicsPatch);
		glFlush();

		free(m_loadedChunks);
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
	

	
	//SDL_UnlockMutex(m_graphicsWrapper->gMutex);
	SDL_UnlockMutex(m_mutex);
}

void ResourceManager::Update(float _dt)
{
	for (int n = 0; n < m_loadedChunksN; ++n)
		m_loadedChunks[n].Popularity += 1;
}

void ResourceManager::DumpCurrentMemory()
{
	SDL_Log("###	Resource Manager	###\n");
	//	Dump currently loaded chunks
	SDL_Log("Chunks\n");
	SDL_Log("Index - Position - Last Seen - IsRendered\n");
	for (int n = 0; n < m_loadedChunksN; ++n)
	{
		LoadedChunk tChunk = m_loadedChunks[n];
		SDL_Log("%i - [%i, %i] - %i - %s\n", n, tChunk.X, tChunk.Z, tChunk.Popularity, tChunk.GraphicsPatch.IsActive ? "Yes" : "No");

	}
	
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
	SDL_LockMutex(m_mutex);
	for (int n = 0; n < m_loadedChunksN; ++n)
		if (m_loadedChunks[n].X == x && m_loadedChunks[n].Z == y)
		{
			m_loadedChunks[n].Popularity = SDL_GetTicks();
			SDL_UnlockMutex(m_mutex);
			//printf("EARLY EXIT: %d,%d\n", x, y);
			return;
		}

	SDL_Point point;
	point.x = x;
	point.y = y;
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
	m_graphicsWrapper = &Graphics::GraphicsWrapper::GetInstance();

	m_hDC = m_graphicsWrapper->GetHDC();

	m_resourceContext = wglCreateContext(m_hDC);

	wglMakeCurrent(m_hDC, m_resourceContext);

	if (wglShareLists(m_resourceContext, m_graphicsWrapper->GetHGLRC()) == FALSE)
		printf("ShareLists error: %i", GetLastError());

	SDL_CondSignal(m_graphicsWrapper->gCond);
	printf("	R Thread: SHARE LIST OK\n");

	printf("	R Thread: WAIT BIG INIT...\n");
	SDL_CondWait(m_graphicsWrapper->gCond, m_graphicsWrapper->gMutex);
	printf("	R Thread: BIG INIT OK\n");
	printf("	R Thread: RUNNING\n");

	
	SDL_UnlockMutex(m_graphicsWrapper->gMutex);
	LoadedChunk* chunk = new LoadedChunk();
	while (true)
	{
		
		SDL_LockMutex(m_mutex);
		if (!m_chunksToPreload.empty())
		{
			//SDL_LockMutex(m_graphicsWrapper->gMutex);
			SDL_Point chunkToLoad = m_chunksToPreload.front();
			m_chunksToPreload.pop();
			//SDL_UnlockMutex(m_mutex);
			
			//Ladda chunk
			//LoadChunk(chunkToLoad.x, chunkToLoad.y);
			//wglMakeCurrent(m_hDC, m_resourceContext);
			m_graphicsWrapper->LoadSingleTexturePatch(chunkToLoad.x, chunkToLoad.y, &chunk->GraphicsPatch, &m_hDC, &m_resourceContext);
			//printf("loaded: %d,%d\n", chunkToLoad.x, chunkToLoad.y);
			//SDL_UnlockMutex(m_graphicsWrapper->gMutex);
			chunk->X = chunkToLoad.x;
			chunk->Z = chunkToLoad.y;
			chunk->Popularity = SDL_GetTicks();

			//SDL_LockMutex(m_mutex);
			m_preloadedChunks.push_back(*chunk);
			
			//SDL_UnlockMutex(m_graphicsWrapper->gMutex);
		}
		SDL_UnlockMutex(m_mutex);
		if (stop)
		{
			SDL_UnlockMutex(m_mutex);
			delete chunk;
			return;
		}
		

		SDL_LockMutex(m_mutex);
		if (!m_preloadedChunks.empty())
		{
			LoadedChunk*	chunkToOverwrite = &m_loadedChunks[GetLeastPopularChunkIndex()];
			if (chunkToOverwrite)
			{
				LoadedChunk* loadedChunk = &m_preloadedChunks.back();
				m_graphicsWrapper->DeleteSingleTexturePatch(&chunkToOverwrite->GraphicsPatch, &m_hDC, &m_resourceContext);
				memcpy(chunkToOverwrite, loadedChunk, sizeof(LoadedChunk));
				m_preloadedChunks.pop_back();
			}
		}
		SDL_UnlockMutex(m_mutex);
	}
	delete chunk;



	return;
}
