#include "CoreECSModule.h"

#include "AssetMasterFile.gen.h"
#include "ECSMasterFile.gen.h"
#include "ShaderMasterRegistryFile.gen.h"
#include "ECS/Ecs.h"

namespace LE
{

void CoreECSModule::RegisterServices() {}

void CoreECSModule::ShutdownServices() {}

void CoreECSModule::RegisterReflection()
{
	AutoRegistration::CoreECSModule::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	AutoRegistration::CoreECSModule::RegisterAllSystems(GetECSModule().GetSystemRegistry());
	AutoRegistration::CoreECSModule::RegisterAllMaterialShader();
}
}
