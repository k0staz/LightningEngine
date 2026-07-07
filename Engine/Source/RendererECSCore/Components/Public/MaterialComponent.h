#pragma once
#include "AssetManager/AssetManager.h"
#include "Assets/MaterialInstanceAsset.h"
#include "ECS/EcsComponent.h"
#include "Math/LinearColor.h"

namespace LE
{
struct MaterialComponent
{
    MaterialComponent() = default;

    AssetHandle<MaterialInstanceAsset> AssetHandle;

    // TODO: This should be separated into a separate component
    LinearColor Color;
    bool IsColor = false;

};

ECS_REGISTER_COMPONENT(MaterialComponent, "MaterialComponent")
}
