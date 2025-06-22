#pragma once

#include "ShaderParameterTypeDescriptors.h"
#include "MeshConverters/MeshConverter.h"

namespace LE::Renderer
{
struct MeshElement;
class MeshDrawSingleShaderBindings;
}

namespace LE::Renderer
{
BEGIN_GLOBAL_CONSTANT_BUFFER(StaticMeshConverterGlobalConstantBuffer)
	DECLARE_GLOBAL_SHADER_PARAMETER_READ_VIEW(VertexFetch_PositionBuffer)
	DECLARE_GLOBAL_SHADER_PARAMETER_READ_VIEW(VertexFetch_TangentBuffer)
	DECLARE_GLOBAL_SHADER_PARAMETER_READ_VIEW(VertexFetch_TexCoordBuffer)
END_CONSTANT_BUFFER()

class StaticMeshConverter : public MeshConverter
{
	DECLARE_MESH_CONVERTER_TYPE()
public:
	struct StaticMeshData
	{
		VertexInputStreamSegment PositionSegment;
		VertexInputStreamSegment TangentSegment;
		VertexInputStreamSegment TextureSegment;

		RHI::RHIReadView* PositionSegmentView = nullptr;
		RHI::RHIReadView* TangentSegmentView = nullptr;
		RHI::RHIReadView* TextureSegmentView = nullptr;
	};

	StaticMeshConverter() = default;

	void InitRHI(RenderCommandList& CommandList) override;
	void SetData(const StaticMeshData& InData);

	virtual void GetShaderBindings(MeshDrawSingleShaderBindings& ShaderBindings, const Shader* Shader, const MeshElement& MeshElement) const override;

	inline RHI::RHIReadView* GetPositionReadView() const;
	inline RHI::RHIReadView* GetTangentReadView() const;
	inline RHI::RHIReadView* GetTextureReadView() const;

protected:
	void InitVertexStreams();

protected:
	StaticMeshData Data;
	ConstantBufferRef<StaticMeshConverterGlobalConstantBuffer> ConstantBuffer;

};

RHI::RHIReadView* StaticMeshConverter::GetPositionReadView() const
{
	return Data.PositionSegmentView;
}

RHI::RHIReadView* StaticMeshConverter::GetTangentReadView() const
{
	return Data.TangentSegmentView;
}

RHI::RHIReadView* StaticMeshConverter::GetTextureReadView() const
{
	return Data.TextureSegmentView;
}
}
