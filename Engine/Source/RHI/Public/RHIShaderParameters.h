#pragma once
#include "CoreDefinitions.h"
#include "RHIResources.h"
#include "Containers/Array.h"

namespace LE::RHI
{
struct RHIShaderParameter
{
	RHIShaderParameter(uint16 InBufferIndex, uint16 InBaseIndex, uint16 InByteOffset, uint16 InByteSize)
		: BufferIndex(InBaseIndex)
		  , BaseIndex(InBaseIndex)
		  , ByteOffset(InByteOffset)
		  , ByteSize(InByteSize)
	{
	}

	uint16 BufferIndex;
	uint16 BaseIndex;
	uint16 ByteOffset;
	uint16 ByteSize;
};

struct RHIShaderParameterResource
{
	enum class ResourceType : uint8
	{
		Texture,
		ReadView,
		WriteView,
		Sampler,
		ConstantBuffer,
	};

	RHIShaderParameterResource() = default;

	RHIShaderParameterResource(ResourceType InType, RHIResource* InResource, uint16 InIndex)
		: Resource(InResource)
		  , Index(InIndex)
		  , Type(InType)
	{
	}

	RHIShaderParameterResource(RHITexture* InResource, uint16 InIndex)
		: Resource(InResource)
		  , Index(InIndex)
		  , Type(ResourceType::Texture)
	{
	}

	RHIShaderParameterResource(RHIReadView* InResource, uint16 InIndex)
		: Resource(InResource)
		, Index(InIndex)
		, Type(ResourceType::ReadView)
	{
	}

	RHIShaderParameterResource(RHIWriteView* InResource, uint16 InIndex)
		: Resource(InResource)
		, Index(InIndex)
		, Type(ResourceType::WriteView)
	{
	}

	RHIShaderParameterResource(RHISamplerState* InResource, uint16 InIndex)
		: Resource(InResource)
		, Index(InIndex)
		, Type(ResourceType::Sampler)
	{
	}

	RHIShaderParameterResource(RHIConstantBuffer* InResource, uint16 InIndex)
		: Resource(InResource)
		, Index(InIndex)
		, Type(ResourceType::ConstantBuffer)
	{
	}

	RHIResource* Resource = nullptr;
	uint16 Index = 0;
	ResourceType Type = ResourceType::Texture;
};

struct RHIShaderParametersCollection
{
	bool HasAnyParameter() const
	{
		return Parameters.size() > 0 + ResourceParameters.size();
	}

	void Reset()
	{
		ParametersData.clear();
		Parameters.clear();
		ResourceParameters.clear();
	}

	void SetShaderParameter(uint32 BufferIndex, uint32 BaseIndex, uint32 ByteSize, const void* Value)
	{
		const uint32 dataOffset = ParametersData.Count();
		ParametersData.insert(ParametersData.end(), static_cast<const uint8*>(Value), static_cast<const uint8*>(Value) + ByteSize);
		Parameters.emplace_back(static_cast<uint16>(BufferIndex), static_cast<uint16>(BaseIndex), static_cast<uint16>(dataOffset), static_cast<uint16>(ByteSize));
	}

	void SetShaderConstantBuffer(uint32 Index, RHIConstantBuffer* ConstantBuffer)
	{
		ResourceParameters.emplace_back(ConstantBuffer, static_cast<uint16>(Index));
	}

	void SetShaderTexture(uint32 Index, RHITexture* Texture)
	{
		ResourceParameters.emplace_back(Texture, static_cast<uint16>(Index));
	}

	void SetShaderReadView(uint32 Index, RHIReadView* ReadView)
	{
		ResourceParameters.emplace_back(ReadView, static_cast<uint16>(Index));
	}

	void SetShaderWriteView(uint32 Index, RHIWriteView* WriteView)
	{
		ResourceParameters.emplace_back(WriteView, static_cast<uint16>(Index));
	}

	void SetShaderSampler(uint32 Index, RHISamplerState* Sampler)
	{
		ResourceParameters.emplace_back(Sampler, static_cast<uint16>(Index));
	}

	Array<uint8> ParametersData;
	Array<RHIShaderParameter> Parameters;
	Array<RHIShaderParameterResource> ResourceParameters;
};
}
