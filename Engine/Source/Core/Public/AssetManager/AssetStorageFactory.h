#pragma once
#include <functional>
#include <map>

#include "AssetCore.h"
#include "AssetStorage.h"

namespace LE
{
class AssetStorageFactory : public ServiceBase
{
public:
    using ReturnType = SharedPtr<AssetStorageBase<AssetTypeId>>;
    using FactoryFunction = std::function<ReturnType()>;

    bool Register(const AssetTypeId TypeId, FactoryFunction Function);
    ReturnType Create(const AssetTypeId TypeId);
    void Initialize() override{}
    void Shutdown() override{}

private:
    std::map<AssetTypeId, FactoryFunction> FactoryFunctions;
};

REGISTER_SERVICE_TYPE(AssetStorageFactory, "AssetStorageFactory")

#define ASSET_STORAGE_CONSTRUCTION_FUNC(Type) \
    []() \
    { \
    return std::make_shared<AssetStorage<AssetTypeId, Type>>(); \
    }
}
