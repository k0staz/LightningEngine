#include "RendererECSCoreModule.h"

#include "AssetMasterFile.gen.h"
#include "ECSMasterFile.gen.h"
#include "ShaderMasterRegistryFile.gen.h"
#include "AssetManager/AssetStorageFactory.h"
#include "ECS/Ecs.h"
#include "Service/ServiceRegistry.h"

namespace LE
{

void RendererEcsCoreModule::RegisterServices() {}

void RendererEcsCoreModule::ShutdownServices() {}

void RendererEcsCoreModule::RegisterReflection()
{
	AutoRegistration::RendererECSCoreModule::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	AutoRegistration::RendererECSCoreModule::RegisterAllSystems(GetECSModule().GetSystemRegistry());
	AutoRegistration::RendererECSCoreModule::RegisterAllMaterialShader();
}
}
