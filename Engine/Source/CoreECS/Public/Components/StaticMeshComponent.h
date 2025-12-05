#pragma once
#include "AssetManager/AssetManager.h"
#include "Assets/StaticMeshAsset.h"
#include "ECS/EcsComponent.h"
#include "StaticMesh/StaticMeshRendering.h"

namespace LE
{
struct StaticMeshComponent
{
	StaticMeshComponent() = default;

	AssetHandle<StaticMeshAsset> AssetHandle;
	Renderer::StaticMeshRenderData* RenderData = nullptr;
	Renderer::Material* MeshMaterial = nullptr;
};

ECS_REGISTER_COMPONENT(StaticMeshComponent, "StaticMeshComponent")
}
