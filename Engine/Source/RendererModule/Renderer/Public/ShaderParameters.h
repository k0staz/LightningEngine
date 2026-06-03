#pragma once
#include "CoreDefinitions.h"

namespace LE::Renderer
{
class ShaderParameterAllocationMap;
}

namespace LE::Renderer
{
enum class ShaderReflectedParameterType : uint8;

class ShaderResourceParameter
{
public:
	ShaderResourceParameter() = default;

	void Bind(const ShaderParameterAllocationMap& ParameterAllocationMap, const String& ParameterName);

	bool IsBound() const { return ResourcesNum > 0; }

	uint16 GetBaseIndex() const { return BaseIndex; }
	uint16 GetResourcesNum() const { return ResourcesNum; }
	ShaderReflectedParameterType GetType() const { return Type; }

private:
	uint16 BaseIndex = 0;
	uint16 ResourcesNum = 0;
	ShaderReflectedParameterType Type;
};

class ShaderConstantBufferParameter
{
public:
	ShaderConstantBufferParameter() = default;

	void Bind(const ShaderParameterAllocationMap& ParameterAllocationMap, const String& ParameterName);

	bool IsBound() const { return BaseIndex != 0xffff; }

	uint16 GetBaseIndex() const { return BaseIndex; }

private:
	uint16 BaseIndex = 0xffff;
};
}
