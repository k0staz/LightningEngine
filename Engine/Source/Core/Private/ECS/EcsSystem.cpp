#include "ECS/EcsSystem.h"

namespace LE
{
	EcsSystemManager::EcsSystemManager()
	{
	}

	void EcsSystemManager::Update()
	{
		for (auto& system : Systems)
		{
			system->Update();
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

