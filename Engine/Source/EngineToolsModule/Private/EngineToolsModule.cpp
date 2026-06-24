#include "EngineToolsModule.h"

#include "AssetMasterFile.gen.h"
#include "ECSMasterFile.gen.h"
#include "FBXImporter.h"
#include "ShaderMasterRegistryFile.gen.h"
#include "ECS/Ecs.h"

namespace LE
{

void EngineToolsModule::RegisterServices()
{
	ServiceRegistry& registry = GetServiceRegistry();
	registry.RegisterService<FBXImporter>();
}

void EngineToolsModule::ShutdownServices()
{
	ServiceRegistry& registry = GetServiceRegistry();
	registry.UnregisterService<FBXImporter>();
}

void EngineToolsModule::RegisterReflection()
{
	AutoRegistration::EngineToolsModule::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	AutoRegistration::EngineToolsModule::RegisterAllSystems(GetECSModule().GetSystemRegistry());
}
}
