#include "ECS/EcsSystem.h"

namespace LE
{

	void EcsSystemManager::Update(const float DeltaSeconds)
	{
		for (auto& system : Systems)
		{
			system->Update(DeltaSeconds);
		}
	}

	void EcsSystemManager::Shutdown()
	{
		for (auto& system : Systems)
		{
			system->Shutdown();
		}
	}
}

