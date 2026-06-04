#pragma once
#include "MeshGroup.h"
#include "RenderDefines.h"
#include "RHIShaderParameters.h"
#include "Materials/Material.h"
#include "Templates/Alignment.h"

namespace LE::Renderer
{
class SceneView;

// A wrapper around ShaderParametersMapInfo to calculate offsets
class MeshDrawShaderBindingsLayout
{
public:
	MeshDrawShaderBindingsLayout(const RefCountingPtr<Shader>& Shader)
		: ParametersMapInfo(&Shader->ReflectionParametersMapInfo)
	{
		LE_ASSERT(Shader.IsValid())
	}

	uint16 GetDataSizeBytes() const
	{
		uint16 dataSize = static_cast<uint16>(sizeof(void*)) * static_cast<uint16>((ParametersMapInfo->ConstantBuffers.Count() + ParametersMapInfo->TextureSamplers.Count() +
			ParametersMapInfo->ReadOnlyViews.Count()));
		dataSize += (ParametersMapInfo->ReadOnlyViews.Count() + 8 - 1) / 8;

		return LE::Align(dataSize, sizeof(void*));
	}

protected:
	inline uint32 GetConstantBufferOffset() const
	{
		return 0;
	}

	inline uint32 GetSamplerOffset() const
	{
		return ParametersMapInfo->ConstantBuffers.Count() * sizeof(RHI::RHIConstantBuffer*);
	}

	inline uint32 GetReadViewOffset() const
	{
		return GetSamplerOffset() + ParametersMapInfo->TextureSamplers.Count() * sizeof(RHI::RHISamplerState*);
	}

	inline uint32 GetReadViewTypeOffset() const
	{
		return GetReadViewOffset() + ParametersMapInfo->ReadOnlyViews.Count() * sizeof(RHI::RHIReadView*);
	}

protected:
	const ShaderParametersMapInfo* ParametersMapInfo;
};

class MeshDrawSingleShaderBindings : public MeshDrawShaderBindingsLayout
{
public:
	MeshDrawSingleShaderBindings(const MeshDrawShaderBindingsLayout& Layout, uint8* InData)
		: MeshDrawShaderBindingsLayout(Layout)
		  , Data(InData)
	{
	}

	template<typename ConstantBufferType>
	void Add(const Shader* Shader, const RHI::RHIConstantBuffer* Value)
	{
		Add(Shader->GetConstantBufferParameter<ConstantBufferType>(), Value);
		AddResources(Shader, Value);
	}

	void AddResources(const Shader* Shader, const RHI::RHIConstantBuffer* ConstantBuffer);

	// Constant Buffer
	void Add(const ShaderConstantBufferParameter& Parameter, const RHI::RHIConstantBuffer* Value);

	// Resources
	void AddReadView(const ShaderResourceParameter& Parameter, const RHI::RHIReadView* Value);
	void AddTexture(const ShaderResourceParameter& Parameter, const RHI::RHITexture* Texture);
	void AddSampler(const ShaderResourceParameter& , const RHI::RHISamplerState* SamplerState);

	void WriteBindingConstantBuffer(const RHI::RHIConstantBuffer* Value, uint16 BaseIndex);
	void WriteBindingReadView(const RHI::RHIReadView* Value, uint16 BaseIndex);
	void WriteBindingTexture(const RHI::RHITexture* Value, uint16 BaseIndex);
	void WriteBindingSampler(const RHI::RHISamplerState* Value, uint16 BaseIndex);

private:
	const RHI::RHIConstantBuffer** GetConstantBufferStart() const
	{
		return reinterpret_cast<const RHI::RHIConstantBuffer**>(Data + GetConstantBufferOffset());
	}

	const RHI::RHISamplerState** GetSamplerStart() const
	{
		return reinterpret_cast<const RHI::RHISamplerState**>(Data + GetSamplerOffset());
	}

	const RHI::RHIResource** GetReadViewStart() const
	{
		return reinterpret_cast<const RHI::RHIResource**>(Data + GetReadViewOffset());
	}

	uint8* GetReadViewTypeStart() const
	{
		return Data + GetReadViewTypeOffset();
	}

private:
	uint8* Data;

	friend class MeshDrawShaderBindings;
};

class ReadOnlyMeshDrawSingleShaderBindings : public MeshDrawShaderBindingsLayout
{
public:
	ReadOnlyMeshDrawSingleShaderBindings(const MeshDrawShaderBindingsLayout& Layout, const uint8* InData)
		: MeshDrawShaderBindingsLayout(Layout)
		, Data(InData)
	{
	}

	RHI::RHIConstantBuffer*const* GetConstantBufferStart() const
	{
		return reinterpret_cast<RHI::RHIConstantBuffer* const*>(Data + GetConstantBufferOffset());
	}

	RHI::RHISamplerState*const* GetSamplerStart() const
	{
		return reinterpret_cast<RHI::RHISamplerState*const*>(Data + GetSamplerOffset());
	}

	RHI::RHIResource*const* GetReadViewStart() const
	{
		return reinterpret_cast<RHI::RHIResource*const*>(Data + GetReadViewOffset());
	}

	const uint8* GetReadViewTypeStart() const
	{
		return Data + GetReadViewTypeOffset();
	}

private:
	const uint8* Data;
	friend class MeshDrawShaderBindings;
};

class MeshDrawShaderBindings
{
public:
	MeshDrawShaderBindings() = default;
	MeshDrawShaderBindings(MeshDrawShaderBindings&& Other) noexcept;
	MeshDrawShaderBindings(const MeshDrawShaderBindings& Other) noexcept;
	MeshDrawShaderBindings& operator=(MeshDrawShaderBindings&& Other) noexcept;
	MeshDrawShaderBindings& operator=(const MeshDrawShaderBindings& Other) noexcept;
	~MeshDrawShaderBindings();

	void Copy(const MeshDrawShaderBindings& Other);
	void Initialize(const ResolvedMaterialShaderMapping& MaterialShaderMapping);

	MeshDrawSingleShaderBindings GetSingleShaderBindings(RHI::ShaderType ShaderType, int32& DataOffset);

	void SetOnCommandList(RenderCommandList& CmdList, const RHI::BoundShadersState& BoundShaders) const;

private:
	Array<MeshDrawShaderBindingsLayout> ShaderBindingsLayouts;

	struct DataStruct
	{
		uint8* InlineStorage[5] = {};

		uint8* GetHeapData()
		{
			return InlineStorage[0];
		}

		const uint8* GetHeapData() const
		{
			return InlineStorage[0];
		}

		void SetHeapData(uint8* HeapData)
		{
			InlineStorage[0] = HeapData;
		}
	} Data = {};

	uint16 ShaderTypeBits = 0;
	uint16 Size = 0;

	bool UsingInlineStorage() const
	{
		return Size <= sizeof(DataStruct);
	}

	uint8* GetData()
	{
		return UsingInlineStorage() ? reinterpret_cast<uint8*>(&Data.InlineStorage[0]) : Data.GetHeapData();
	}

	const uint8* GetData() const
	{
		return UsingInlineStorage() ? reinterpret_cast<const uint8*>(&Data.InlineStorage[0]) : Data.GetHeapData();
	}

	void Allocate(uint16 InSize)
	{
		LE_ASSERT(Size == 0 && Data.GetHeapData() == nullptr)

		Size = InSize;

		if (InSize > sizeof(Data))
		{
			Data.SetHeapData(new uint8[InSize]);
		}
	}

	void AllocateZeroed(uint16 InSize)
	{
		Allocate(InSize);

		if (!UsingInlineStorage())
		{
			MemsetZero(GetData(), InSize);
		}
	}

	void Release();

	static void SetShaderBindings(RHI::RHIShaderParametersCollection& ParametersCollection,
	                              const ReadOnlyMeshDrawSingleShaderBindings& SingleShaderBindings);
};

class MeshDrawCommand
{
public:
	static bool SubmitDrawBegin(const MeshDrawCommand& Command, RenderCommandList& CmdList);
	static bool SubmitDrawEnd(const MeshDrawCommand& Command, RenderCommandList& CmdList);

	RHI::PipelineStateInitializer PipelineStateInitializer;
	MeshDrawShaderBindings ShaderBindings;
	RHI::RHIBuffer* IndexBuffer;
	uint32 StencilRef;
	RHI::PrimitiveType PrimitiveType;
	uint32 PrimitiveCount;

	uint32 FirstIndex;
	struct
	{
		uint32 BaseVertexIndex;
	} VertexParams;
};

class MeshPassRenderState
{
public:
	MeshPassRenderState() = default;
	MeshPassRenderState(const MeshPassRenderState&) = default;
	~MeshPassRenderState() = default;

	void SetDepthStencilState(RHI::RHIDepthStencilState* InDepthStencilState)
	{
		DepthStencilState = InDepthStencilState;
		StencilRef = 0;
	}

	void SetStencilRef(uint32 InStencilRef)
	{
		StencilRef = InStencilRef;
	}

	void SetDepthStencilAccess(RHI::ExclusiveDepthStencil InDepthStencilAccess)
	{
		DepthStencilAccess = InDepthStencilAccess;
	}

	RHI::RHIDepthStencilState* GetDepthStencilState() const
	{
		return DepthStencilState;
	}

	uint32 GetStencilRef() const
	{
		return StencilRef;
	}

	RHI::ExclusiveDepthStencil GetDepthStencilAccess() const
	{
		return DepthStencilAccess;
	}

private:
	RHI::RHIDepthStencilState* DepthStencilState = nullptr;
	RHI::ExclusiveDepthStencil DepthStencilAccess = RHI::ExclusiveDepthStencil::DepthRead_StencilRead;
	uint32 StencilRef = 0;
};

class MessPassCommandBuilder
{
public:
	void BuildMeshDrawCommands(const MeshGroup& MeshGroup, const SceneView* SceneView);

	RenderPassType PassType;
	MeshPassRenderState PassRenderState;
	Array<MeshDrawCommand> DrawList;
};
}
