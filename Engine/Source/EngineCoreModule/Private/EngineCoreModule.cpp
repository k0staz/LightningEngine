#include "EngineCoreModule.h"

#include "AssetManager/AssetManager.h"
#include "Service/ServiceRegistry.h"

#include "AssetMasterFile.gen.h"
#include "ECSMasterFile.gen.h"
#include "ShaderMasterRegistryFile.gen.h"
#include "ECS/Ecs.h"
#include "EventCore/EventManager.h"

void LE::EngineCoreModule::RegisterServices()
{
	ServiceRegistry& serviceRegistry = GetServiceRegistry();
	serviceRegistry.RegisterService<AssetManager>();
	serviceRegistry.RegisterService<AssetRegistry>();
	serviceRegistry.RegisterService<AssetStorageFactory>();
	serviceRegistry.RegisterService<EventManager>();
}

void LE::EngineCoreModule::ShutdownServices()
{
	ServiceRegistry& serviceRegistry = GetServiceRegistry();
	serviceRegistry.UnregisterService<AssetManager>();
	serviceRegistry.UnregisterService<AssetRegistry>();
	serviceRegistry.UnregisterService<AssetStorageFactory>();
	serviceRegistry.UnregisterService<EventManager>();
}

void LE::EngineCoreModule::RegisterReflection()
{
	AutoRegistration::EngineCoreModule::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	AutoRegistration::EngineCoreModule::RegisterAllSystems(GetECSModule().GetSystemRegistry());
	AutoRegistration::EngineCoreModule::RegisterAllMaterialShader();
}
