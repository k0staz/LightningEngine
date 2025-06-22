#pragma once
#include "RenderResource.h"
#include "Templates/RefCounters.h"


namespace LE::Renderer
{
enum class ShaderReflectedParameterType : uint8
{
	Sampler,
	ReadOnlyView,
	ReadWriteView,
	ConstantBuffer,

	Num
};

struct ShaderParameterAllocation
{
	uint16 BufferIndex = 0;
	uint16 BaseIndex = 0;
	uint16 Size = 0;
	ShaderReflectedParameterType Type{ ShaderReflectedParameterType::Num};
	mutable bool IsBound = false;

	ShaderParameterAllocation() = default;

	ShaderParameterAllocation(uint16 InBufferIndex, uint16 InBaseIndex, uint16 InSize, ShaderReflectedParameterType InType)
		: BufferIndex(InBufferIndex)
		  , BaseIndex(InBaseIndex)
		  , Size(InSize)
		  , Type(InType)
	{
	}

	friend bool operator==(const ShaderParameterAllocation& Left, const ShaderParameterAllocation& Right)
	{
		return Left.BufferIndex == Right.BufferIndex
			&& Left.BaseIndex == Right.BaseIndex
			&& Left.Size == Right.Size
			&& Left.Type == Right.Type;
	}

	friend bool operator!=(const ShaderParameterAllocation& Left, const ShaderParameterAllocation& Right)
	{
		return !(Left == Right);
	}
};

class ShaderParameterAllocationMap
{
public:
	ShaderParameterAllocationMap() = default;
	Optional<ShaderParameterAllocation> GetParameterAllocation(const String& ParameterName) const;
	bool GetParameterAllocation(const String& ParameterName, uint16& OutBufferIndex, uint16& OutBaseIndex, uint16& OutSize) const;
	bool HasParameterAllocation(const String& ParameterName) const;
	void AddParameterAllocation(const char* ParameterName, uint16 BufferIndex, uint16 BaseIndex, uint16 Size,
		ShaderReflectedParameterType ParameterType);
	void RemoveParameterAllocation(const char* ParameterName);

	Array<String> GetAllParameterNamesOfType(ShaderReflectedParameterType InType) const;

	const Map<String, ShaderParameterAllocation>& GetParametersMap() const { return ParametersMap; }

private:
	Map<String, ShaderParameterAllocation> ParametersMap;
};
}
