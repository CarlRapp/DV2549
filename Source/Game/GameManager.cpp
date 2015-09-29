#include "GameManager.h"
#include "Graphics/GraphicsWrapper.h"

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
	int currentX = (int)(currentPos.x / 16);
	int currentZ = (int)(currentPos.z / 16);

	if(currentX != m_oldPosX || currentZ != m_oldPosZ)
	{
		int sizeX = m_graphicsWrapper->GetLevel()->X / 2;
		int sizeZ = m_graphicsWrapper->GetLevel()->Y / 2;

		for (int X = -m_tileRenderDistance; X <= m_tileRenderDistance; ++X)
		{
			for (int Z = -m_tileRenderDistance; Z <= m_tileRenderDistance; ++Z)
			{
				if (std::abs(currentX + X) < sizeX && std::abs(currentZ + Z) < sizeZ)
				{
					m_graphicsWrapper->LoadSingleTexturePatch(currentX + X, currentZ + Z);
				}
					
			}
		}

		m_oldPosX	=	currentX;
		m_oldPosZ	=	currentZ;

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



}