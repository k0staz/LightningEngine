#include "AutoRegistration.h"

#include "AssetMasterFile.gen.h"
#include "ShaderMasterRegistryFile.gen.h"
#include "ECS/Ecs.h"

namespace LE
{
void RegisterAutoTypes()
{
	AutoRegistration::EngineModule::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	//AutoRegistration::RegisterAllSystems(*GetECSModule().GetSystemManager()); TODO: This needs to be added after System Refactoring
	Renderer::AutoRegistration::EngineModule::RegisterAllMaterialShader(); 
}
}
