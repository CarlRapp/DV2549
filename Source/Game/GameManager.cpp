#include "GameManager.h"
#include "Graphics/GraphicsWrapper.h"
#include "ResourceManager/ResourceManager.h"

GameManager::GameManager() :
	m_oldPosX(0), m_oldPosY(0), m_oldPosZ(0), m_tileRenderDistance(1)
{
	
}
GameManager::~GameManager()
{
}

GameManager& GameManager::GetInstance()
{
	static GameManager* m_instance = new GameManager();
	return *m_instance;
}

void GameManager::Update(float dt)
{
	//	Check if the camera is in another 
	//	tile position than last frame
	glm::vec3	currentPos	= m_graphicsWrapper->GetCamera()->GetPosition();
	int currentX = (int)(currentPos.x / m_graphicsWrapper->GetLevel()->PatchSize);
	int currentZ = (int)(currentPos.z / m_graphicsWrapper->GetLevel()->PatchSize);

	if(currentX != m_oldPosX || currentZ != m_oldPosZ)
	{
		m_oldPosX	=	currentX;
		m_oldPosZ	=	currentZ;

		LoadSurroundingChunks();
	}


}

void GameManager::SetRenderDistance(unsigned int _chunkDistance)
{
	//	Divide the requested render distance
	//	since we go from -m_tileRenderDistance to m_tileRenderDistance
	m_tileRenderDistance	= _chunkDistance <= 1 ? 1 :_chunkDistance/2;

	//	Delete the current pool from the resourcemanager
	unsigned int numberOfChunks = (_chunkDistance*2+1)*(_chunkDistance * 2 + 1);


	ResourceManager::GetInstance().CreateChunkPool(numberOfChunks);
	glm::vec3	currentPos = m_graphicsWrapper->GetCamera()->GetPosition();
	int currentX = (int)(currentPos.x / m_graphicsWrapper->GetLevel()->PatchSize);
	int currentZ = (int)(currentPos.z / m_graphicsWrapper->GetLevel()->PatchSize);

	if (currentX != m_oldPosX || currentZ != m_oldPosZ)
	{
		m_oldPosX = currentX;
		m_oldPosZ = currentZ;

		LoadSurroundingChunks();
	}

	LoadSurroundingChunks();
}

void GameManager::LoadSurroundingChunks()
{

	int sizeX = m_graphicsWrapper->GetLevel()->X / 2;
	int sizeZ = m_graphicsWrapper->GetLevel()->Y / 2;

	for (int X = -m_tileRenderDistance; X <= m_tileRenderDistance; ++X)
	{
		for (int Z = -m_tileRenderDistance; Z <= m_tileRenderDistance; ++Z)
		{
			if (std::abs(m_oldPosX + X) < sizeX && std::abs(m_oldPosZ + Z) < sizeZ)
			{
				ResourceManager::GetInstance().LoadChunk(m_oldPosX + X, m_oldPosZ + Z);
				//m_graphicsWrapper->LoadSingleTexturePatch(m_oldPosX + X, m_oldPosZ + Z);
			}
		}
	}

}
