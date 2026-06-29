#pragma once
#include "AssetManager/AssetManager.h"
#include "Assets/StaticMeshAsset.h"
#include "ECS/EcsComponent.h"

namespace LE
{
struct StaticMeshComponent
{
	StaticMeshComponent() = default;

	AssetHandle<StaticMeshAsset> AssetHandle;
};

ECS_REGISTER_COMPONENT(StaticMeshComponent, "StaticMeshComponent")
}
