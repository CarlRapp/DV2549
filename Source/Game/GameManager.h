#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#ifdef WIN32
#ifdef _DEBUG
#include <VLD/vld.h>
#endif
#else
#endif
#include <SDL/SDL.h>
#include "ResourceManager/ResourceManager.h"

	class GameManager
	{
	public:
		~GameManager();
		static GameManager& GetInstance();
		void SetGraphicsWrapper(Graphics::GraphicsWrapper* instance){ m_graphicsWrapper = instance; }

		void Update(float dt);

		void SetRenderDistance(unsigned int _chunkDistance);
		int GetRenderDistance() { return m_tileRenderDistance; }

	private:
		void LoadSurroundingChunks();

	private:
		GameManager();
		Graphics::GraphicsWrapper*	m_graphicsWrapper;

		int m_oldPosX, m_oldPosY, m_oldPosZ;
		int m_tileRenderDistance;
	};

#endif