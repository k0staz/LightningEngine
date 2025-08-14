#include "ECS/Ecs.h"

#include "Log.h"


namespace LE
{
static UniquePtr<ECSModule> GEcsModule;

void RegisterECSModule(UniquePtr<ECSModule> Module)
{
	GEcsModule = std::move(Module);
}

ECSModule& GetECSModule()
{
	if (!GEcsModule)
	{
		LE_ERROR("ECS Module is not registered");
	}

	return *GEcsModule;
}
}
