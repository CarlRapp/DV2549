#include "GameManager.h"
#include "Graphics/GraphicsWrapper.h"
#include "ResourceManager/ResourceManager.h"

GameManager::GameManager() :
	m_oldPosX(-999), m_oldPosY(0), m_oldPosZ(-999), m_tileRenderDistance(1)
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
	int currentX = (int)(currentPos.x / 16);
	int currentZ = (int)(currentPos.z / 16);

	if(currentX != m_oldPosX || currentZ != m_oldPosZ)
	{
		m_oldPosX	=	currentX;
		m_oldPosZ	=	currentZ;

		LoadSurroundingChunks();
	}

/*
int x =  m_level.X / 2;
int y =  m_level.Y / 2;

// 	//Add individual patch data, like different heightmap
for (int i = -x; i < x; i++)
{
for (int j = -y; j < y; j++)
{
TerrainPatch* newItem = new TerrainPatch;

//newItem->TextureDiffuse = LoadTextureRAW("../../../Content/test.raw", 512, 512, 3);
newItem->TextureHeight = LoadTexturePatch("../../../Content/height.pak", j + m_level.Y/2, i+m_level.X/2, 1);
newItem->TextureNormal = LoadTexturePatch("../../../Content/norm.pak", j + m_level.Y / 2, i + m_level.X / 2, 3);
newItem->TextureDiffuse = LoadTexturePatch("../../../Content/diffuse.pak", j + m_level.Y / 2, i + m_level.X / 2, 3);
newItem->ModelMatrix = glm::translate(glm::vec3(i*m_patchSize, 0, j*m_patchSize));

m_terrainPatches.push_back(newItem);
}
}
*/



}

void GameManager::SetRenderDistance(unsigned int _chunkDistance)
{
	//	Divide the requested render distance
	//	since we go from -m_tileRenderDistance to m_ti
	m_tileRenderDistance	=	_chunkDistance/2;

	//	Delete the current pool from the resourcemanager
	ResourceManager::GetInstance().CreateChunkPool(6, 6);

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
				m_graphicsWrapper->LoadSingleTexturePatch(m_oldPosX + X, m_oldPosZ + Z);
		}
	}

}
