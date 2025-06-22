#pragma once
#include "ECS/EcsComponent.h"
#include "StaticMesh/StaticMeshRendering.h"

namespace LE
{
class StaticMeshComponent : public EcsComponent
{
public:
	COMPONENT_CLASS("StaticMeshComponent")

	StaticMeshComponent(EntityId OwnerId)
		: EcsComponent(OwnerId)
	{
	}

	Renderer::StaticMeshRenderData* RenderData;
	Renderer::Material* MeshMaterial;
};
}
