#include "MeshPassCommandBuilders/MeshPassCommandBuilder.h"

#include "DynamicRHI.h"
#include "RenderCommandList.h"
#include "ShaderManager.h"
#include "ShaderParameters.h"
#include "Containers/Array.h"
#include "MeshConverters/MeshConverter.h"

namespace LE::Renderer
{
void MeshDrawSingleShaderBindings::AddResources(const Shader* Shader, const RHI::RHIConstantBuffer* ConstantBuffer)
{
	const RHI::RHIConstantBufferLayout& layout = ConstantBuffer->GetLayout();
	const Array<RefCountingPtr<RHI::RHIResource>>& resources = ConstantBuffer->GetResources();

	for (uint32 index = 0; index < resources.Count(); ++index)
	{
		const RHI::RHIConstantBufferResource& bufferResource = layout.Resources[index];
		if (bufferResource.ResourceType == RHI::SPT_Buffer_ReadView 
			|| bufferResource.ResourceType == RHI::SPT_Global_ReadView 
			|| bufferResource.ResourceType == RHI::SPT_Texture_ReadView)
		{
			RHI::RHIReadView* readView = dynamic_cast<RHI::RHIReadView*>(resources[index].GetPointer());
			AddReadView(Shader->GetResourceParameter(bufferResource.Name.c_str()), readView);
		}
		else if (bufferResource.ResourceType == RHI::SPT_Texture)
		{
			RHI::RHITexture* texture = dynamic_cast<RHI::RHITexture*>(resources[index].GetPointer());
			AddTexture(Shader->GetResourceParameter(bufferResource.Name.c_str()), texture);
		}
		else if (bufferResource.ResourceType == RHI::SPT_Sampler)
		{
			RHI::RHISamplerState* sampler = dynamic_cast<RHI::RHISamplerState*>(resources[index].GetPointer());
			AddSampler(Shader->GetResourceParameter(bufferResource.Name.c_str()), sampler);
		}
		else
		{
			LE_ERROR("Unhandled shader resource");
		}
	}
}

void MeshDrawSingleShaderBindings::Add(const ShaderConstantBufferParameter& Parameter, const RHI::RHIConstantBuffer* Value)
{
	if (Parameter.IsBound())
	{
		WriteBindingConstantBuffer(Value, Parameter.GetBaseIndex());
	}
}

void MeshDrawSingleShaderBindings::AddReadView(const ShaderResourceParameter& Parameter, const RHI::RHIReadView* Value)
{
	if (Parameter.IsBound())
	{
		WriteBindingReadView(Value, Parameter.GetBaseIndex());
	}
}

void MeshDrawSingleShaderBindings::AddTexture(const ShaderResourceParameter& Parameter, const RHI::RHITexture* Texture)
{
	if (Parameter.IsBound())
	{
		WriteBindingTexture(Texture, Parameter.GetBaseIndex());
	}
}

void MeshDrawSingleShaderBindings::AddSampler(const ShaderResourceParameter& Parameter, const RHI::RHISamplerState* SamplerState)
{
	if (Parameter.IsBound())
	{
		WriteBindingSampler(SamplerState, Parameter.GetBaseIndex());
	}
}

void MeshDrawSingleShaderBindings::WriteBindingConstantBuffer(const RHI::RHIConstantBuffer* Value, uint16 BaseIndex)
{
	const auto& iterator = std::find_if(ParametersMapInfo->ConstantBuffers.begin(), ParametersMapInfo->ConstantBuffers.end(),
	                                    [&BaseIndex](const ShaderConstantBufferParameterInfo& item)
	                                    {
		                                    return item.BaseIndex == BaseIndex;
	                                    });

	if (iterator == ParametersMapInfo->ConstantBuffers.end())
	{
		LE_ERROR("Trying to bind something which doesn't belong to this shader");
		return;
	}

	const uint32 index = static_cast<uint32>(iterator - ParametersMapInfo->ConstantBuffers.begin());

	GetConstantBufferStart()[index] = Value;
}

void MeshDrawSingleShaderBindings::WriteBindingReadView(const RHI::RHIReadView* Value, uint16 BaseIndex)
{
	const auto& iterator = std::find_if(ParametersMapInfo->ReadOnlyViews.begin(), ParametersMapInfo->ReadOnlyViews.end(),
	                                    [&BaseIndex](const ShaderResourceParameterInfo& item)
	                                    {
		                                    return item.BaseIndex == BaseIndex;
	                                    });

	if (iterator == ParametersMapInfo->ReadOnlyViews.end())
	{
		LE_ERROR("Trying to bind something which doesn't belong to this shader");
		return;
	}

	const uint32 index = static_cast<uint32>(iterator - ParametersMapInfo->ReadOnlyViews.begin());

	uint32 typeByteIndex = index / 8;
	uint32 typeBitIndex = index % 8;
	GetReadViewTypeStart()[typeByteIndex] |= 1 << typeBitIndex;
	GetReadViewStart()[index] = Value;
}

void MeshDrawSingleShaderBindings::WriteBindingTexture(const RHI::RHITexture* Value, uint16 BaseIndex)
{
	const auto& iterator = std::find_if(ParametersMapInfo->ReadOnlyViews.begin(), ParametersMapInfo->ReadOnlyViews.end(),
	                                    [&BaseIndex](const ShaderResourceParameterInfo& item)
	                                    {
		                                    return item.BaseIndex == BaseIndex;
	                                    });

	if (iterator == ParametersMapInfo->ReadOnlyViews.end())
	{
		LE_ERROR("Trying to bind something which doesn't belong to this shader");
		return;
	}
	const uint32 index = static_cast<uint32>(iterator - ParametersMapInfo->ReadOnlyViews.begin());
	GetReadViewStart()[index] = Value;
}

void MeshDrawSingleShaderBindings::WriteBindingSampler(const RHI::RHISamplerState* Value, uint16 BaseIndex)
{
	const auto& iterator = std::find_if(ParametersMapInfo->TextureSamplers.begin(), ParametersMapInfo->TextureSamplers.end(),
	                                    [&BaseIndex](const ShaderResourceParameterInfo& item)
	                                    {
		                                    return item.BaseIndex == BaseIndex;
	                                    });

	if (iterator == ParametersMapInfo->TextureSamplers.end())
	{
		LE_ERROR("Trying to bind something which doesn't belong to this shader");
		return;
	}
	const uint32 index = static_cast<uint32>(iterator - ParametersMapInfo->TextureSamplers.begin());
	GetSamplerStart()[index] = Value;
}

MeshDrawShaderBindings::MeshDrawShaderBindings(MeshDrawShaderBindings&& Other) noexcept
{
	Release();

	Size = Other.Size;
	ShaderTypeBits = Other.ShaderTypeBits;
	ShaderBindingsLayouts = std::move(Other.ShaderBindingsLayouts);

	if (Other.UsingInlineStorage())
	{
		Data = std::move(Other.Data);
	}
	else
	{
		Data.SetHeapData(Other.Data.GetHeapData());
		Other.Data.SetHeapData(nullptr);
	}
	Other.Size = 0;
}

MeshDrawShaderBindings::MeshDrawShaderBindings(const MeshDrawShaderBindings& Other) noexcept
{
	Copy(Other);
}

MeshDrawShaderBindings& MeshDrawShaderBindings::operator=(MeshDrawShaderBindings&& Other) noexcept
{
	Release();

	Size = Other.Size;
	ShaderTypeBits = Other.ShaderTypeBits;
	ShaderBindingsLayouts = std::move(Other.ShaderBindingsLayouts);

	if (Other.UsingInlineStorage())
	{
		Data = std::move(Other.Data);
	}
	else
	{
		Data.SetHeapData(Other.Data.GetHeapData());
		Other.Data.SetHeapData(nullptr);
	}
	Other.Size = 0;

	return *this;
}

MeshDrawShaderBindings& MeshDrawShaderBindings::operator=(const MeshDrawShaderBindings& Other) noexcept
{
	Copy(Other);
	return *this;
}

MeshDrawShaderBindings::~MeshDrawShaderBindings()
{
	Release();
}

void MeshDrawShaderBindings::Copy(const MeshDrawShaderBindings& Other)
{
	Release();

	ShaderTypeBits = Other.ShaderTypeBits;
	ShaderBindingsLayouts = Other.ShaderBindingsLayouts;

	Allocate(Other.Size);

	if (Other.UsingInlineStorage())
	{
		Data = Other.Data;
	}
	else
	{
		std::memcpy(GetData(), Other.GetData(), Size);
	}
}

void MeshDrawShaderBindings::Initialize(const ResolvedMaterialShaderMapping& MaterialShaderMapping)
{
	const RefCountingPtr<Shader> vertexShader = MaterialShaderMapping.VertexShaderMapping->GetLogicalShader();
	const RefCountingPtr<Shader> pixelShader = MaterialShaderMapping.PixelShaderMapping->GetLogicalShader();

	const int8 shadersNumber = (vertexShader.IsValid() ? 1 : 0)
		+ (pixelShader.IsValid() ? 1 : 0);

	ShaderBindingsLayouts.clear();
	ShaderBindingsLayouts.reserve(shadersNumber);

	uint16 bindingsDataSize = 0;

	if (vertexShader.IsValid())
	{
		ShaderBindingsLayouts.emplace_back(vertexShader);
		bindingsDataSize += ShaderBindingsLayouts.back().GetDataSizeBytes();
		ShaderTypeBits |= (1 << static_cast<uint8>(RHI::ShaderType::Vertex));
	}

	if (pixelShader.IsValid())
	{
		ShaderBindingsLayouts.emplace_back(pixelShader);
		bindingsDataSize += ShaderBindingsLayouts.back().GetDataSizeBytes();
		ShaderTypeBits |= (1 << static_cast<uint8>(RHI::ShaderType::Pixel));
	}

	if (bindingsDataSize > 0)
	{
		AllocateZeroed(bindingsDataSize);
	}
}

MeshDrawSingleShaderBindings MeshDrawShaderBindings::GetSingleShaderBindings(RHI::ShaderType ShaderType, int32& DataOffset)
{
	int32 shaderTypeIndex = LE::CountBits(ShaderTypeBits & ((1 << (static_cast<uint8>(ShaderType) + 1)) - 1)) - 1;

	if (shaderTypeIndex >= 0)
	{
		int32 startDataOffset = DataOffset;
		DataOffset += ShaderBindingsLayouts[shaderTypeIndex].GetDataSizeBytes();
		return {ShaderBindingsLayouts[shaderTypeIndex], GetData() + startDataOffset};
	}

	LE_ASSERT(false)
	return {{RefCountingPtr<Shader>(nullptr)}, nullptr};
}

void MeshDrawShaderBindings::SetOnCommandList(RenderCommandList& CmdList, const RHI::BoundShadersState& BoundShaders) const
{
	const uint8* bindingDataPtr = GetData();
	uint16 shaderTypeBitIndex = ~0;
	for (uint32 shaderBindingIndex = 0; shaderBindingIndex < ShaderBindingsLayouts.Count(); ++shaderBindingIndex)
	{
		RHI::ShaderType shaderType = RHI::ShaderType::Count;
		while (true)
		{
			++shaderTypeBitIndex;
			if ((ShaderTypeBits & (1 << shaderTypeBitIndex)) != 0)
			{
				shaderType = static_cast<RHI::ShaderType>(shaderTypeBitIndex);
				break;
			}
		}
		LE_ASSERT(shaderType < RHI::ShaderType::Count)

		ReadOnlyMeshDrawSingleShaderBindings singleShaderBindings(ShaderBindingsLayouts[shaderBindingIndex], bindingDataPtr);

		if (shaderType == RHI::ShaderType::Vertex)
		{
			RHI::RHIShaderParametersCollection& parametersCollection = CmdList.GetScratchShaderParametersCollection();
			SetShaderBindings(parametersCollection, singleShaderBindings);
			CmdList.SetShaderParametersCollection(BoundShaders.VertexShaderRHI, parametersCollection);
		}
		if (shaderType == RHI::ShaderType::Pixel)
		{
			RHI::RHIShaderParametersCollection& parametersCollection = CmdList.GetScratchShaderParametersCollection();
			SetShaderBindings(parametersCollection, singleShaderBindings);
			CmdList.SetShaderParametersCollection(BoundShaders.PixelShaderRHI, parametersCollection);
		}

		bindingDataPtr += ShaderBindingsLayouts[shaderBindingIndex].GetDataSizeBytes();
	}
}

void MeshDrawShaderBindings::Release()
{
	if (!UsingInlineStorage())
	{
		delete[] Data.GetHeapData();
	}

	Size = 0;
	Data.SetHeapData(nullptr);
}

void MeshDrawShaderBindings::SetShaderBindings(RHI::RHIShaderParametersCollection& ParametersCollection,
                                               const ReadOnlyMeshDrawSingleShaderBindings& SingleShaderBindings)
{
	RHI::RHIConstantBuffer** constantBufferBindings = const_cast<RHI::RHIConstantBuffer**>(SingleShaderBindings.GetConstantBufferStart());
	const Array<ShaderConstantBufferParameterInfo>& bufferParameterInfo = SingleShaderBindings.ParametersMapInfo->ConstantBuffers;
	const uint32 constBufferNum = SingleShaderBindings.ParametersMapInfo->ConstantBuffers.Count();

	for (uint32 index = 0; index < constBufferNum; ++index)
	{
		const ShaderConstantBufferParameterInfo& parameter = bufferParameterInfo[index];
		if (RHI::RHIConstantBuffer* constantBuffer = constantBufferBindings[index])
		{
			ParametersCollection.SetShaderConstantBuffer(parameter.BaseIndex, constantBuffer);
		}
	}

	RHI::RHISamplerState* const* samplerBindings = SingleShaderBindings.GetSamplerStart();
	const Array<ShaderResourceParameterInfo>& samplerParameters = SingleShaderBindings.ParametersMapInfo->TextureSamplers;
	for (uint32 index = 0; index < samplerParameters.Count(); ++index)
	{
		const ShaderResourceParameterInfo& parameter = samplerParameters[index];
		if (RHI::RHISamplerState* sampler = samplerBindings[index])
		{
			ParametersCollection.SetShaderSampler(parameter.BaseIndex, sampler);
		}
	}

	const uint8* readViewType = SingleShaderBindings.GetReadViewTypeStart();
	RHI::RHIResource* const* readViewBindings = SingleShaderBindings.GetReadViewStart();
	const Array<ShaderResourceParameterInfo>& viewParameters = SingleShaderBindings.ParametersMapInfo->ReadOnlyViews;
	for (uint32 index = 0; index < viewParameters.Count(); ++index)
	{
		const ShaderResourceParameterInfo& parameter = viewParameters[index];

		uint32 typeByteIndex = index / 8;
		uint32 typeBitIndex = index % 8;

		if (readViewType[typeByteIndex] & (1 << typeBitIndex))
		{
			if (RHI::RHIReadView* readView = dynamic_cast<RHI::RHIReadView*>(readViewBindings[index]))
			{
				ParametersCollection.SetShaderReadView(parameter.BaseIndex, readView);
			}
		}
		else
		{
			if (RHI::RHITexture* texture = dynamic_cast<RHI::RHITexture*>(readViewBindings[index]))
			{
				ParametersCollection.SetShaderTexture(parameter.BaseIndex, texture);
			}
		}
	}
}

bool MeshDrawCommand::SubmitDrawBegin(const MeshDrawCommand& Command, RenderCommandList& CmdList)
{
	// TODO:: This needs to be reworked so we cache PSO, also right now this pso will be deleted once out of scope of this function,
	// but this is alright for D3D11, but needs to be addressed in the future with cache
	RefCountingPtr<RHI::RHIPipelineStateObject> pso = RHI::RHICreatePipelineStateObject(Command.PipelineStateInitializer);
	CmdList.SetGraphicsPSO(pso, Command.StencilRef);

	Command.ShaderBindings.SetOnCommandList(CmdList, Command.PipelineStateInitializer.ShaderState);

	return true;
}

bool MeshDrawCommand::SubmitDrawEnd(const MeshDrawCommand& Command, RenderCommandList& CmdList)
{
	if (Command.IndexBuffer)
	{
		CmdList.DrawIndexedPrimitive(Command.IndexBuffer, Command.VertexParams.BaseVertexIndex, Command.FirstIndex, Command.PrimitiveCount);
	}

	return true;
}

void MessPassCommandBuilder::BuildMeshDrawCommands(const MeshGroup& MeshGroup, const SceneView* SceneView)
{
	MeshDrawCommand& drawCommand = DrawList.emplace_back();

	const Material* material = MeshGroup.MeshMaterial->GetMaterial();
	if (!material)
	{
		LE_ASSERT(false)
		return;
	}

	const Material::ShaderSet* shaderSet = material->GetShaderSetForRenderPass(PassType);
	if (!shaderSet)
	{
		LE_ASSERT(false)
		return;
	}

	ResolvedMaterialShaderMapping shaderMappings = material->GetShaderMappings(PassType, MeshGroup.MeshConverter->GetMeshConverterType());

	RefCountingPtr<ShaderMapping> vertexMapping = shaderMappings.VertexShaderMapping;
	RefCountingPtr<ShaderMapping> pixelMapping = shaderMappings.PixelShaderMapping;

	drawCommand.StencilRef = PassRenderState.GetStencilRef();
	drawCommand.PrimitiveType = MeshGroup.PrimitiveType;

	RHI::PipelineStateInitializer& pipelineStateInitializer = drawCommand.PipelineStateInitializer;
	pipelineStateInitializer.Primitive = MeshGroup.PrimitiveType;

	pipelineStateInitializer.ShaderState.VertexShaderRHI = vertexMapping->GetShaderRHI();
	pipelineStateInitializer.ShaderState.PixelShaderRHI = pixelMapping->GetShaderRHI();

	pipelineStateInitializer.DepthStencilState = PassRenderState.GetDepthStencilState();
	pipelineStateInitializer.DepthStencilAccess = PassRenderState.GetDepthStencilAccess();

	drawCommand.PrimitiveCount = MeshGroup.Element.PrimitivesCount;
	drawCommand.IndexBuffer = MeshGroup.Element.IndexBuffer->IndexBufferRHI.GetPointer();
	drawCommand.FirstIndex = MeshGroup.Element.FirstIndex;
	drawCommand.VertexParams.BaseVertexIndex = MeshGroup.Element.BaseVertexIndex;

	drawCommand.ShaderBindings.Initialize(shaderMappings);

	const RefCountingPtr<Shader> vertexShader = vertexMapping->GetLogicalShader();
	const RefCountingPtr<Shader> pixelShader = pixelMapping->GetLogicalShader();

	int32 dataOffset = 0;
	if (vertexShader.IsValid())
	{
		MeshDrawSingleShaderBindings shaderBindings = drawCommand.ShaderBindings.GetSingleShaderBindings(
			RHI::ShaderType::Vertex, dataOffset);

		MeshGroup.MeshConverter->GetShaderBindings(shaderBindings, vertexShader.GetPointer(), MeshGroup.Element);
		MeshGroup.MeshMaterial->GetShaderBindings(&shaderBindings, vertexShader.GetPointer(),
		                                          (*shaderSet)[static_cast<uint32>(RHI::ShaderType::Vertex)], SceneView);
	}

	if (pixelShader.IsValid())
	{
		MeshDrawSingleShaderBindings shaderBindings = drawCommand.ShaderBindings.GetSingleShaderBindings(
			RHI::ShaderType::Pixel, dataOffset);

		MeshGroup.MeshConverter->GetShaderBindings(shaderBindings, pixelShader.GetPointer(), MeshGroup.Element);
		MeshGroup.MeshMaterial->GetShaderBindings(&shaderBindings, pixelShader.GetPointer(),
		                                          (*shaderSet)[static_cast<uint32>(RHI::ShaderType::Pixel)], SceneView);
	}
}
}
