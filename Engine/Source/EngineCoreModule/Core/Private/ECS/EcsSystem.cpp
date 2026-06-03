#include "ECS/EcsSystem.h"

namespace LE
{
	void EcsSystemRegistry::Shutdown()
	{
		for (auto& system : Systems)
		{
			system->Shutdown();
		}
	}
}

