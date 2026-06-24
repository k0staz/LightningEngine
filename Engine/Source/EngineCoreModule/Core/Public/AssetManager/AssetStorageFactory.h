#pragma once
#include <functional>
#include <map>

#include "AssetCore.h"
#include "AssetStorage.h"
#include "AssetRegistry.h"

namespace LE
{
class AssetStorageFactory : public ServiceBase
{
public:
	using ReturnType = SharedPtr<AssetStorageBase<AssetTypeId>>;
	using FactoryFunction = std::function<ReturnType()>;
	
	using CreateFunction = std::function<bool(Asset&, AssetInfo&)>;

	bool Register(const AssetTypeId TypeId, FactoryFunction Function, CreateFunction CreateF);
	ReturnType Create(const AssetTypeId TypeId);
	bool Load(AssetInfo& Info, Asset& Asset);

	void Initialize() override {}

	void Shutdown() override {}

private:
	struct AssetTypeInfo
	{
		FactoryFunction FactoryFunction;
		CreateFunction CreateFunction;
	};
	
	std::map<AssetTypeId, AssetTypeInfo> FactoryFunctions;
};

REGISTER_SERVICE_TYPE(AssetStorageFactory, "AssetStorageFactory")

#define ASSET_STORAGE_CONSTRUCTION_FUNC(Type) \
    []() \
    { \
    return std::make_shared<AssetStorage<AssetTypeId, Type>>(); \
    }

#define ASSET_STORAGE_LOAD_FUNC(Type) \
	[](Asset& Asset, AssetInfo& Info) \
	{ \
		Type& asset = static_cast<Type&>(Asset); \
        std::vector<std::byte> assetData; \
		bool success = LoadFile(Info.PathToAsset, assetData); \
		if (success) \
		{ \
			Archive::Context context(&Info); \
			Archive::ArchiveReader reader(assetData); \
			success = Archive::Deserialize(context, reader, asset); \
		} \
		return success;\
	}
}
