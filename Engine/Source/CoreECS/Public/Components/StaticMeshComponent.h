#pragma once
#include "ECS/EcsComponent.h"
#include "StaticMesh/StaticMeshRendering.h"

namespace LE
{
struct StaticMeshComponent
{
	StaticMeshComponent() = default;

	Renderer::StaticMeshRenderData* RenderData = nullptr;
	Renderer::Material* MeshMaterial = nullptr;
};

ECS_REGISTER_COMPONENT(StaticMeshComponent, "StaticMeshComponent")
}
