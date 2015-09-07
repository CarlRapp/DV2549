#include "MemoryWrapper.h"


using namespace Memory;
#pragma region Constructor/Destructor and GetInstance
MemoryWrapper::MemoryWrapper()
{

}
MemoryWrapper::~MemoryWrapper()
{
}

MemoryWrapper& MemoryWrapper::GetInstance()
{
	static MemoryWrapper* m_instance = new MemoryWrapper();
	return *m_instance;
}
#pragma endregion


