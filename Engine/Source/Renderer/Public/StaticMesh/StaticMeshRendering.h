#pragma once

#include "CoreDefinitions.h"
#include "Math/Vector3.h"
#include "MeshConverters/StaticMeshConverter.h"
#include "Multithreading/SharedResource.h"
#include "SceneRendering/RenderObjectProxy.h"


namespace LE::Renderer
{
struct StaticMeshVertex
{
	Vector4F Position;

	Vector3F Tangent;

	Vector2F TextureCord;
};

class StaticMeshVertexBuffers : public RenderResource
{
public:
	StaticMeshVertexBuffers();

	void Init(uint32 InNumVertices);

	void Init(const Array<StaticMeshVertex>& InVertices);

	void InitRHI(RenderCommandList& CommandList) override;

	void BindPositionVertexBuffer(StaticMeshConverter::StaticMeshData& Data);
	void BindTangentVertexBuffer(StaticMeshConverter::StaticMeshData& Data);
	void BindTexCoordBuffer(StaticMeshConverter::StaticMeshData& Data);

	Vector4F GetVertexPosition(uint32 Index) const;
	Vector3F GetVertexTangent(uint32 Index) const;
	Vector2F GetVertexTexCoord(uint32 Index) const;
	uint32 GetNumVertices() const { return NumVertices; }

	bool IsValid() const
	{
		return PositionBuffer.VertexBufferRHI.IsValid()
		&& TangentBuffer.VertexBufferRHI.IsValid()
		&& TexCoordBuffer.VertexBufferRHI.IsValid();
	}

	VertexBuffer PositionBuffer;
	VertexBuffer TangentBuffer;
	VertexBuffer TexCoordBuffer;

private:
	RefCountingPtr<RHI::RHIBuffer> CreatePositionRHIBuffer();
	RefCountingPtr<RHI::RHIBuffer> CreateTangentRHIBuffer();
	RefCountingPtr<RHI::RHIBuffer> CreateTexCoordRHIBuffer();

private:
	ResourceArray<Vector4F> PositionData;
	RefCountingPtr<RHI::RHIReadView> PositionReadView;

	ResourceArray<Vector3F> TangentData;
	RefCountingPtr<RHI::RHIReadView> TangentReadView;

	ResourceArray<Vector2F> TexCoordData;
	RefCountingPtr<RHI::RHIReadView> TexCoordReadView;

	uint32 NumVertices;
};

class StaticMeshIndexBuffer : public IndexBuffer
{
public:
	StaticMeshIndexBuffer() = default;

	void Init(const Array<uint16>& Indices);
	void InitRHI(RenderCommandList& CommandList) override;
	uint32 GetIndicesCount() const { return IndexCount; }

	RefCountingPtr<RHI::RHIBuffer> CreateIndexRHIBuffer();

private:
	ResourceArray<uint16> IndexData;
	uint32 IndexCount;
};

struct StaticMeshRenderData
{
	void InitResources();

	StaticMeshVertexBuffers VertexBuffers;
	StaticMeshConverter MeshConverter;
	StaticMeshIndexBuffer IndexBuffer;
	RHI::PrimitiveType PrimitiveType;
};

class StaticMeshRenderProxy : public RenderObjectProxy
{
public:
	StaticMeshRenderProxy(EcsEntity InOwnerEntity, const StaticMeshRenderData* InRenderData, RefCountingPtr<MaterialInstance> InMaterial);

	const StaticMeshRenderData* GetRenderData() const { return RenderData; }

	void GetMeshGroup(MeshGroup& OutMeshGroup) override;

private:
	const StaticMeshRenderData* RenderData;
	RefCountingPtr<MaterialInstance> MeshMaterial;
};
};

namespace LE
{
	REGISTER_SHARED_RESOURCE(Renderer::StaticMeshRenderProxy, "StaticMeshRenderProxy")
}
