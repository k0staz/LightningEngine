#include "MeshConverters/StaticMeshConverter.h"

#include "MeshPassCommandBuilders/MeshPassCommandBuilder.h"
#include "SceneRendering/RenderObjectProxy.h"

namespace LE::Renderer
{
IMPLEMENT_GLOBAL_CONSTANT_BUFFER(StaticMeshConverterGlobalConstantBuffer, "StaticMeshConverter")

void StaticMeshConverter::InitRHI(RenderCommandList& CommandList)
{
	InitVertexStreams();

	StaticMeshConverterGlobalConstantBuffer constantBuffer;

	constantBuffer.VertexFetch_PositionBuffer = GetPositionReadView();
	constantBuffer.VertexFetch_TangentBuffer = GetTangentReadView();
	constantBuffer.VertexFetch_TexCoordBuffer = GetTextureReadView();

	ConstantBuffer = ConstantBufferRef<StaticMeshConverterGlobalConstantBuffer>::CreateConstantBuffer(constantBuffer);
}

void StaticMeshConverter::SetData(const StaticMeshData& InData)
{
	Data = InData;
}

void StaticMeshConverter::GetShaderBindings(MeshDrawSingleShaderBindings& ShaderBindings, const Shader* Shader, const MeshElement& MeshElement) const
{
	ShaderBindings.Add<StaticMeshConverterGlobalConstantBuffer>(Shader, ConstantBuffer.GetPointer());

	if (MeshElement.ObjectConstantBuffer)
	{
		ShaderBindings.Add<ObjectShaderParameters>(Shader, MeshElement.ObjectConstantBuffer);
	}
}

void StaticMeshConverter::InitVertexStreams()
{
	if (Data.PositionSegment.VertexBuffer != nullptr)
	{
		Streams.emplace_back(Data.PositionSegment.Offset, Data.PositionSegment.Stride, Data.PositionSegment.VertexBuffer);
	}

	if (Data.TangentSegment.VertexBuffer != nullptr)
	{
		Streams.emplace_back(Data.TangentSegment.Offset, Data.TangentSegment.Stride, Data.TangentSegment.VertexBuffer);
	}

	if (Data.TextureSegment.VertexBuffer != nullptr)
	{
		Streams.emplace_back(Data.TangentSegment.Offset, Data.TangentSegment.Stride, Data.TangentSegment.VertexBuffer);
	}
}

// TODO:: Add actual HLSL filename which contains code related to this MC
IMPLEMENT_MESH_CONVERTER_TYPE(StaticMeshConverter, "BlaBlaBla")
}
