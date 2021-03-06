#include "GameManager.h"
#include "Graphics/GraphicsWrapper.h"
#include "ResourceManager/ResourceManager.h"

static GameManager* m_instance = nullptr;

GameManager::GameManager() :
	m_oldPosX(0), m_oldPosY(0), m_oldPosZ(0), m_tileRenderDistance(1), m_oldTileRenderDistance(1)
{
	
}
GameManager::~GameManager()
{
}

GameManager& GameManager::GetInstance()
{
	if (m_instance == nullptr)
		m_instance = new GameManager();
	return *m_instance;
}

void GameManager::Update(float dt)
{
	//	Check if the camera is in another 
	//	tile position than last frame
	glm::vec3	currentPos	= m_graphicsWrapper->GetCamera()->GetPosition();

	float offset = m_graphicsWrapper->GetLevel()->PatchSize/2.0f;

	int currentX = (int)std::roundf((currentPos.x - offset) / m_graphicsWrapper->GetLevel()->PatchSize);
	int currentZ = (int)std::roundf((currentPos.z - offset) / m_graphicsWrapper->GetLevel()->PatchSize);

	if(currentX != m_oldPosX || currentZ != m_oldPosZ)
	{
		printf("New position: \n");
		printf("%d,%d\n", currentX, currentZ);
		printf("%.0f,%.0f\n", currentPos.x, currentPos.z);

		m_oldPosX	=	currentX;
		m_oldPosZ	=	currentZ;

		//ChangeRenderDistance();
		LoadSurroundingChunks();
	}

	if (m_oldTileRenderDistance != m_tileRenderDistance)
	{
		ChangeRenderDistance();
	}
}

void GameManager::RequestRenderDistance(unsigned int _chunkDistance)
{
	int tempDistance = m_tileRenderDistance;

	//	Divide the requested render distance
	//	since we go from -m_tileRenderDistance to m_tileRenderDistance
	m_tileRenderDistance = _chunkDistance <= 1 ? 1 : _chunkDistance;
}

void GameManager::ChangeRenderDistance()
{

	//	Delete the current pool from the resourcemanager
	unsigned int numberOfChunks = (m_tileRenderDistance * 2 + 1)*(m_tileRenderDistance * 2 + 1);


	ResourceManager::GetInstance().CreateChunkPool(numberOfChunks);
	glm::vec3	currentPos = m_graphicsWrapper->GetCamera()->GetPosition();

	float offset = m_graphicsWrapper->GetLevel()->PatchSize / 2.0f;

	int currentX = (int)std::roundf((currentPos.x - offset) / m_graphicsWrapper->GetLevel()->PatchSize);
	int currentZ = (int)std::roundf((currentPos.z - offset) / m_graphicsWrapper->GetLevel()->PatchSize);

	if (currentX != m_oldPosX || currentZ != m_oldPosZ)
	{
		m_oldPosX = currentX;
		m_oldPosZ = currentZ;
	}

	LoadSurroundingChunks();

	m_oldTileRenderDistance = m_tileRenderDistance;
}
void GameManager::LoadSurroundingChunks()
{

	int sizeX = m_graphicsWrapper->GetLevel()->X / 2;
	int sizeZ = m_graphicsWrapper->GetLevel()->Y / 2;

	for (int X = -m_tileRenderDistance; X <= m_tileRenderDistance; ++X)
		for (int Z = -m_tileRenderDistance; Z <= m_tileRenderDistance; ++Z)
		{
			
			if (std::abs(m_oldPosX + X) < sizeX && std::abs(m_oldPosZ + Z) < sizeZ)
			{
				ResourceManager::GetInstance().LoadChunk_Threaded(m_oldPosX + X, m_oldPosZ + Z);
			}
			else
				printf("Discarded patch %d,%d\n", X, Z);

		}
	//glFlush();
}
