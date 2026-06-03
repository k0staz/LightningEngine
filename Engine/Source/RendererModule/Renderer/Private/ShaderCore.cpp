#include "ShaderCore.h"

namespace LE::Renderer
{
Optional<ShaderParameterAllocation> ShaderParameterAllocationMap::GetParameterAllocation(const String& ParameterName) const
{
	const auto& iterator = ParametersMap.find(ParameterName);
	if (iterator != ParametersMap.end())
	{
		const ShaderParameterAllocation& allocation = iterator->second;
		allocation.IsBound = true;

		return {allocation};
	}

	return {};
}

bool ShaderParameterAllocationMap::GetParameterAllocation(const String& ParameterName, uint16& OutBufferIndex, uint16& OutBaseIndex,
                                                 uint16& OutSize) const
{
	if (const auto allocation = GetParameterAllocation(ParameterName))
	{
		OutBufferIndex = allocation->BufferIndex;
		OutBaseIndex = allocation->BaseIndex;
		OutSize = allocation->Size;

		return true;
	}

	return false;
}

bool ShaderParameterAllocationMap::HasParameterAllocation(const String& ParameterName) const
{
	const auto& iterator = ParametersMap.find(ParameterName);
	return iterator != ParametersMap.end();
}

void ShaderParameterAllocationMap::AddParameterAllocation(const char* ParameterName, uint16 BufferIndex, uint16 BaseIndex, uint16 Size,
                                                 ShaderReflectedParameterType ParameterType)
{
	LE_ASSERT(ParameterType != ShaderReflectedParameterType::Num)
	ParametersMap[ParameterName] = ShaderParameterAllocation(BufferIndex, BaseIndex, Size, ParameterType);
}

void ShaderParameterAllocationMap::RemoveParameterAllocation(const char* ParameterName)
{
	const auto& iterator = ParametersMap.find(ParameterName);
	if (iterator != ParametersMap.end())
	{
		ParametersMap.erase(iterator);
	}
}

Array<String> ShaderParameterAllocationMap::GetAllParameterNamesOfType(ShaderReflectedParameterType InType) const
{
	Array<String> result;
	for (auto& iterator : ParametersMap)
	{
		if (iterator.second.Type == InType)
		{
			result.push_back(iterator.first);
		}
	}

	return result;
}
}
