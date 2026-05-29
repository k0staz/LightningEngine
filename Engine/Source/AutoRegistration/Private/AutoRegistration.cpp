#include "AutoRegistration.h"

#include "AssetTypesAutoRegistration.h"
#include "ECSSystemAutoRegistration.h"
#include "MaterialShaderAutoRegistration.h"
#include "ECS/Ecs.h"

namespace LE
{
void RegisterAutoTypes()
{
	AutoRegistration::RegisterAllAssetTypes(GetServiceRegistry().GetService<AssetStorageFactory>());
	//AutoRegistration::RegisterAllSystems(*GetECSModule().GetSystemManager()); TODO: This needs to be added after System Refactoring
	Renderer::AutoRegistration::RegisterAllMaterialShader(); 
}
}
