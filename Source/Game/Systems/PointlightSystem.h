#ifndef POINTLIGHTSYSTEM_H
#define POINTLIGHTSYSTEM_H

#include "GraphicalSystem.h"
#if defined(__ANDROID__) || defined(__IOS__)
#include "AndroidRenderer/GraphicDevice.h"
#else
#include "Renderer/GraphicDevice.h"
#endif


class PointlightSystem : public GraphicalSystem
{
public:
	PointlightSystem(Renderer::GraphicDevice* _graphics);
	~PointlightSystem();

	void Initialize();
	void Update(const ECSL::RuntimeInfo& _runtime);

	void EntitiesAdded(const ECSL::RuntimeInfo& _runtime, const std::vector<unsigned int>& _entities);
	void EntitiesRemoved(const ECSL::RuntimeInfo& _runtime, const std::vector<unsigned int>& _entities);

private:
	void UpdatePointLights();

	ECSL::BitSet::DataType* m_bitMask;
	unsigned int m_numberOfBitSets;
	unsigned int m_changedComponentId;
	unsigned int m_pointLightId;;
};

#endif