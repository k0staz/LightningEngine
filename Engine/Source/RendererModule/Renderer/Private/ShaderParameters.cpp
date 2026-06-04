#include "ShaderParameters.h"

#include "ShaderCore.h"

namespace LE::Renderer
{
void ShaderResourceParameter::Bind(const ShaderParameterAllocationMap& ParameterAllocationMap, const String& ParameterName)
{
	if (Optional<ShaderParameterAllocation> allocation = ParameterAllocationMap.GetParameterAllocation(ParameterName))
	{
		BaseIndex = allocation->BaseIndex;
		ResourcesNum = allocation->Size;
		Type = allocation->Type;
	}
}

void ShaderConstantBufferParameter::Bind(const ShaderParameterAllocationMap& ParameterAllocationMap, const String& ParameterName)
{
	if (Optional<ShaderParameterAllocation> allocation = ParameterAllocationMap.GetParameterAllocation(ParameterName))
	{
		BaseIndex = allocation->BufferIndex;
	}
}
}
