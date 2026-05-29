#include "AssetManager/AssetStorageFactory.h"

namespace LE
{

bool AssetStorageFactory::Register(const AssetTypeId TypeId, FactoryFunction Function)
{
	FactoryFunctions[TypeId] = Function;
	return true;
}

AssetStorageFactory::ReturnType AssetStorageFactory::Create(const AssetTypeId TypeId)
{
	if(!FactoryFunctions.contains(TypeId))
	{
		return nullptr;
	}
	
	return FactoryFunctions[TypeId]();
}
}
