#pragma once
#include "CoreDefinitions.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"

namespace LE::RHI
{
struct RHIConstantBufferResourceInitializer
{
	friend bool operator==(const RHIConstantBufferResourceInitializer& Left, const RHIConstantBufferResourceInitializer& Right)
	{
		return Left.ResourceOffset == Right.ResourceOffset && Left.ResourceType == Right.ResourceType;
	}

	String Name;
	uint16 ResourceOffset;
	ShaderParameterType ResourceType;
};

struct RHIConstantBufferInitializer
{
	RHIConstantBufferInitializer() = default;

	RHIConstantBufferInitializer(String InName)
		: Name(InName)
	{
	}

	String Name;

	Array<RHIConstantBufferResourceInitializer> Resources;

	uint32 ConstantBufferSize = 0;

	friend bool operator==(const RHIConstantBufferInitializer& Left, const RHIConstantBufferInitializer& Right)
	{
		return Left.ConstantBufferSize == Right.ConstantBufferSize
			&& Left.Resources == Right.Resources;
	}
};
}
