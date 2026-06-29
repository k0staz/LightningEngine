#pragma once
#include "AssetManager/AssetManager.h"
#include "Assets/MaterialInstanceAsset.h"
#include "ECS/EcsComponent.h"

namespace LE
{
struct MaterialComponent
{
    MaterialComponent() = default;

    AssetHandle<MaterialInstanceAsset> AssetHandle;
};

ECS_REGISTER_COMPONENT(MaterialComponent, "MaterialComponent")
}
