#include "ECS/EcsSystem.h"

namespace LE
{
	void EcsSystemManager::Shutdown()
	{
		for (auto& system : Systems)
		{
			system->Shutdown();
		}
	}
}

