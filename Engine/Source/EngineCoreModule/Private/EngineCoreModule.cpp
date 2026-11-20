#include "EngineCoreModule.h"

#include "AssetManager/AssetManager.h"
#include "Service/ServiceRegistry.h"

#include "AssetMasterFile.gen.h"
#include "ECSMasterFile.gen.h"
#include "SystemManager.h"
#include "AssetManager/MountRegistry.h"
#include "ECS/Ecs.h"
#include "EventCore/EventManager.h"

void LE::EngineCoreModule::RegisterServices()
{
	ServiceRegistry& serviceRegistry = GetServiceRegistry();
	serviceRegistry.RegisterService<MountRegistry>();
	serviceRegistry.RegisterService<AssetManager>();
	serviceRegistry.RegisterService<AssetRegistry>();
	serviceRegistry.RegisterService<AssetStorageFactory>();
	serviceRegistry.RegisterService<EventManager>();
	serviceRegistry.RegisterService<SystemManager>();
}

void LE::EngineCoreModule::ShutdownServices()
{
	ServiceRegistry& serviceRegistry = GetServiceRegistry();
	serviceRegistry.UnregisterService<AssetManager>();
	serviceRegistry.UnregisterService<AssetRegistry>();
	serviceRegistry.UnregisterService<AssetStorageFactory>();
	serviceRegistry.UnregisterService<MountRegistry>();
	serviceRegistry.UnregisterService<EventManager>();
	serviceRegistry.UnregisterService<SystemManager>();
}

void LE::EngineCoreModule::RegisterReflection()
{
	AutoRegistration::EngineCoreModule::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	AutoRegistration::EngineCoreModule::RegisterAllSystems(GetECSModule().GetSystemRegistry());
}
