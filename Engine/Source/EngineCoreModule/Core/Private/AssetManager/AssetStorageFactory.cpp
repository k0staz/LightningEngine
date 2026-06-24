#include "AssetManager/AssetStorageFactory.h"

namespace LE
{

bool AssetStorageFactory::Register(const AssetTypeId TypeId, FactoryFunction Function, CreateFunction CreateF)
{
	FactoryFunctions[TypeId] = {Function, CreateF};
	return true;
}

AssetStorageFactory::ReturnType AssetStorageFactory::Create(const AssetTypeId TypeId)
{
	if(!FactoryFunctions.contains(TypeId))
	{
		return nullptr;
	}
	
	return FactoryFunctions[TypeId].FactoryFunction();
}

bool AssetStorageFactory::Load(AssetInfo& Info, Asset& Asset)
{
	if (!FactoryFunctions.contains(Info.TypeId))
	{
		return false;
	}
	
	return FactoryFunctions[Info.TypeId].CreateFunction(Asset, Info);
}
}
