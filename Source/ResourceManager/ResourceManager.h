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


class DECLSPEC ResourceManager
{
public:
	~ResourceManager();
	static ResourceManager& GetInstance();

	bool InitResourceManager(size_t _totalMemorySize);

	bool LoadAsset();
	bool UnloadAsset();



private:
	ResourceManager();


private:
	size_t	m_currentAllocatedMemory;
	size_t	m_totalMemorySize;
};


#endif