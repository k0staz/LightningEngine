#include "StaticMesh/StaticMeshRendering.h"

#include "RenderCommandList.h"
#include "RHIResources.h"

namespace LE::Renderer
{
namespace 
{
uint32 GetVertexCountForPrimitiveType(RHI::PrimitiveType Type)
{
	switch (Type)
	{
	case RHI::PrimitiveType::TriangleList:
		return 3;
	case RHI::PrimitiveType::TriangleStrip:
		return 1;
	case RHI::PrimitiveType::LineList:
		return 2;
	case RHI::PrimitiveType::PointList:
		return 1;
	case RHI::PrimitiveType::Count:
	default:
		LE_ERROR("Wrong primitive type");
		return 0;
	}
}
}

StaticMeshVertexBuffers::StaticMeshVertexBuffers(): PositionReadView(nullptr), TangentReadView(nullptr), TexCoordReadView(nullptr),
                                                    NumVertices(0)
{
}

void StaticMeshVertexBuffers::Init(uint32 InNumVertices)
{
	NumVertices = InNumVertices;

	PositionData.reserve(InNumVertices);
	TangentData.reserve(InNumVertices);
	TexCoordData.reserve(InNumVertices);
}

void StaticMeshVertexBuffers::Init(const Array<StaticMeshVertex>& InVertices)
{
	Init(InVertices.Count());

	for (const StaticMeshVertex& vertex : InVertices)
	{
		PositionData.emplace_back(vertex.Position);
		TangentData.emplace_back(vertex.Tangent);
		TexCoordData.emplace_back(vertex.TextureCord);
	}
}

void StaticMeshVertexBuffers::InitRHI(RenderCommandList& CommandList)
{
	PositionBuffer.VertexBufferRHI = CreatePositionRHIBuffer();
	if (PositionBuffer.VertexBufferRHI)
	{
		auto initializer = RHI::RHIViewDescription::CreateBufferReadView();
		initializer.SetType(RHI::RHIViewDescription::BufferType::Typed);
		initializer.SetFormat(RHI::PixelFormat::R32G32B32A32_FLOAT);
		PositionReadView = CommandList.CreateReadView(PositionBuffer.VertexBufferRHI, initializer);
	}

	TangentBuffer.VertexBufferRHI = CreateTangentRHIBuffer();
	if (TangentBuffer.VertexBufferRHI)
	{
		auto initializer = RHI::RHIViewDescription::CreateBufferReadView();
		initializer.SetType(RHI::RHIViewDescription::BufferType::Typed);
		initializer.SetFormat(RHI::PixelFormat::R32G32B32A32_FLOAT);
		TangentReadView = CommandList.CreateReadView(TangentBuffer.VertexBufferRHI, initializer);
	}

	TexCoordBuffer.VertexBufferRHI = CreateTexCoordRHIBuffer();
	if (TexCoordBuffer.VertexBufferRHI)
	{
		auto initializer = RHI::RHIViewDescription::CreateBufferReadView();
		initializer.SetType(RHI::RHIViewDescription::BufferType::Typed);
		initializer.SetFormat(RHI::PixelFormat::R32G32_FLOAT);
		TexCoordReadView = CommandList.CreateReadView(TexCoordBuffer.VertexBufferRHI, initializer);
	}
}

void StaticMeshVertexBuffers::BindPositionVertexBuffer(StaticMeshConverter::StaticMeshData& Data)
{
	Data.PositionSegmentView = PositionReadView;

	Data.PositionSegment = VertexInputStreamSegment(&PositionBuffer, RHI::VertexElementType::Float3, 0, 0, sizeof(Vector3F));
}

void StaticMeshVertexBuffers::BindTangentVertexBuffer(StaticMeshConverter::StaticMeshData& Data)
{
	Data.TangentSegmentView = TangentReadView;

	Data.TangentSegment = VertexInputStreamSegment(&TangentBuffer, RHI::VertexElementType::Float3, 0, 0, sizeof(Vector3F));
}

void StaticMeshVertexBuffers::BindTexCoordBuffer(StaticMeshConverter::StaticMeshData& Data)
{
	Data.TextureSegmentView = TexCoordReadView;

	Data.TextureSegment = VertexInputStreamSegment(&TexCoordBuffer, RHI::VertexElementType::Float2, 0, 0, sizeof(Vector2F));
}

Vector4F StaticMeshVertexBuffers::GetVertexPosition(uint32 Index) const
{
	if (Index >= NumVertices)
	{
		LE_ASSERT(false)
		return {};
	}

	return PositionData[Index];
}

Vector3F StaticMeshVertexBuffers::GetVertexTangent(uint32 Index) const
{
	if (Index >= NumVertices)
	{
		LE_ASSERT(false)
		return {};
	}

	return TangentData[Index];
}

Vector2F StaticMeshVertexBuffers::GetVertexTexCoord(uint32 Index) const
{
	if (Index >= NumVertices)
	{
		LE_ASSERT(false)
		return {};
	}

	return TexCoordData[Index];
}

RefCountingPtr<RHI::RHIBuffer> StaticMeshVertexBuffers::CreatePositionRHIBuffer()
{
	return CreateRHIBuffer(&PositionData, RHI::BUF_ShaderResource);
}

RefCountingPtr<RHI::RHIBuffer> StaticMeshVertexBuffers::CreateTangentRHIBuffer()
{
	return CreateRHIBuffer(&TangentData, RHI::BUF_ShaderResource);
}

RefCountingPtr<RHI::RHIBuffer> StaticMeshVertexBuffers::CreateTexCoordRHIBuffer()
{
	return CreateRHIBuffer(&TexCoordData, RHI::BUF_ShaderResource);
}

void StaticMeshIndexBuffer::Init(const Array<uint16>& Indices)
{
	IndexData = Indices;
	IndexCount = Indices.Count();
}

void StaticMeshIndexBuffer::InitRHI(RenderCommandList& CommandList)
{
	IndexBufferRHI = CreateIndexRHIBuffer();
}

RefCountingPtr<RHI::RHIBuffer> StaticMeshIndexBuffer::CreateIndexRHIBuffer()
{
	const uint32 dataSizeInBytes = IndexData.GetResourceDataSize();
	RHI::RHIResourceCreateInfo createInfo(&IndexData);

	return RenderCommandList::Get().CreateIndexBuffer(2 ,dataSizeInBytes, RHI::BUF_Index, createInfo);
}

void StaticMeshRenderData::InitResources()
{
	RenderCommandList::Get().EnqueueLambdaCommand([this](Renderer::RenderCommandList& CmdList)
	{
		IndexBuffer.InitRHI(CmdList);
		VertexBuffers.InitRHI(CmdList);

		StaticMeshConverter::StaticMeshData data;

		VertexBuffers.BindPositionVertexBuffer(data);
		VertexBuffers.BindTangentVertexBuffer(data);
		VertexBuffers.BindTexCoordBuffer(data);

		MeshConverter.SetData(data);

		MeshConverter.InitRHI(CmdList);
	});
}

StaticMeshRenderProxy::StaticMeshRenderProxy(EntityId InOwnerEntityId, const StaticMeshRenderData* InRenderData,
                                             RefCountingPtr<MaterialInstance> InMaterial)
	: RenderObjectProxy(InOwnerEntityId)
	  , RenderData(InRenderData)
	  , MeshMaterial(InMaterial)
{
}

void StaticMeshRenderProxy::GetMeshGroup(MeshGroup& OutMeshGroup)
{
	MeshElement& element = OutMeshGroup.Element;
	element.ObjectConstantBuffer = ConstantBuffer.GetPointer();

	const StaticMeshRenderData& renderData = *RenderData;

	element.IndexBuffer = &renderData.IndexBuffer;
	element.PrimitivesCount = renderData.IndexBuffer.GetIndicesCount() / GetVertexCountForPrimitiveType(renderData.PrimitiveType);

	OutMeshGroup.MeshConverter = &RenderData->MeshConverter;
	OutMeshGroup.MeshMaterial = MeshMaterial;
	OutMeshGroup.PrimitiveType = renderData.PrimitiveType;
}
}
