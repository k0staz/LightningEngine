#pragma once
#include "CoreDefinitions.h"
#include "RenderResource.h"
#include "Materials/Material.h"

namespace LE::Renderer
{
class MeshConverter;
}

namespace LE::Renderer
{
struct MeshElement
{
	RHI::RHIConstantBuffer* ObjectConstantBuffer = nullptr;
	const IndexBuffer* IndexBuffer = nullptr;
	uint32 FirstIndex = 0;
	uint32 BaseVertexIndex = 0;
	uint32 PrimitivesCount = 0;
};

struct MeshGroup
{
	MeshElement Element;
	RHI::PrimitiveType PrimitiveType;
	const MeshConverter* MeshConverter;
	RefCountingPtr<MaterialInstance> MeshMaterial;
};
}
