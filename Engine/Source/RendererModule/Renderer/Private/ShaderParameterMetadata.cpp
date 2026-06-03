#include "ShaderParameterMetadata.h"

#include <algorithm>

#include "DynamicRHI.h"
#include "RHIConstantBufferInitializer.h"
#include "ShaderCore.h"
#include "Misc/EnumFlags.h"

namespace LE::Renderer
{
static LinkedList<ShaderParametersMetadata*>* gShaderParametersLists = nullptr;

LinkedList<ShaderParametersMetadata*>*& ShaderParametersMetadata::GetParametersList()
{
	if (!gShaderParametersLists)
	{
		gShaderParametersLists = new LinkedList<ShaderParametersMetadata*>();
	}

	return gShaderParametersLists;
}

ShaderParametersMetadata::ShaderParametersMetadata(
	Type InType,
	const char* InConstantBufferName,
	const char* InFileName,
	uint32 InSize,
	const Array<Parameter>& InParameters)
	: ConstantBufferName(InConstantBufferName)
	  , FileName(InFileName)
	  , Size(InSize)
	  , ParametersType(InType)
	  , Parameters(InParameters)
{
	if (InType == Type::GlobalBuffer)
	{
		GetParametersList()->push_back(this);
	}

	Initialize(nullptr);
}


class ConstantBufferElementAndOffset
{
public:
	ConstantBufferElementAndOffset(const ShaderParametersMetadata* InParentParameter,
	                               const ShaderParametersMetadata::Parameter* InParameter,
	                               uint32 InParameterOffset)
		: ParentParameter(InParentParameter)
		  , Parameter(InParameter)
		  , ParameterOffset(InParameterOffset)
	{
	}

	const ShaderParametersMetadata* ParentParameter;
	const ShaderParametersMetadata::Parameter* Parameter;
	uint32 ParameterOffset;
};

const ShaderParametersMetadata::Parameter* ShaderParametersMetadata::GetParameter(const String& ParameterName) const
{
	for (const Parameter& parameter : Parameters)
	{
		if (strcmp(parameter.GetName(), ParameterName.c_str()) == 0)
		{
			return &parameter;
		}
	}

	return nullptr;
}

void ShaderParametersMetadata::Initialize(RHI::RHIConstantBufferInitializer* OutBufferInitializer)
{
	LE_ASSERT(!IsLayoutInitialized())

	RHI::RHIConstantBufferInitializer localInitializer(ConstantBufferName);
	RHI::RHIConstantBufferInitializer& initializer = OutBufferInitializer ? *OutBufferInitializer : localInitializer;
	initializer.ConstantBufferSize = Size;

	Array<ConstantBufferElementAndOffset> parameterStack;
	parameterStack.reserve(Parameters.size());
	for (const auto& element : Parameters)
	{
		parameterStack.emplace_back(this, &element, 0);
	}

	for (size_t i = 0; i < parameterStack.size(); ++i)
	{
		const ShaderParametersMetadata& currentParameter = *parameterStack[i].ParentParameter;
		const Parameter& currentElement = *parameterStack[i].Parameter;

		RHI::ShaderParameterType type = currentElement.GetShaderParameterType();
		const uint32 arraySize = currentElement.GetElementsNum();

		const bool isArray = arraySize > 0;
		if (!currentElement.IsParameterNativeType())
		{
			for (uint32 arrayElementId = 0; arrayElementId < (isArray ? arraySize : 1u); ++arrayElementId)
			{
				const uint32 absoluteElementOffset = currentElement.GetOffset() + parameterStack[arrayElementId].ParameterOffset +
					arrayElementId * SHADER_PARAMETER_POINTER_ALIGNMENT;
				const RHI::RHIConstantBufferResourceInitializer resourceParameter{
					currentElement.GetName(), static_cast<uint16>(absoluteElementOffset), type
				};

				initializer.Resources.push_back(resourceParameter);
			}
		}
	}

	const auto sortByElementOffset = [](const RHI::RHIConstantBufferResourceInitializer& Left,
	                                    const RHI::RHIConstantBufferResourceInitializer& Right)
	{
		return Left.ResourceOffset < Right.ResourceOffset;
	};

	std::sort(initializer.Resources.begin(), initializer.Resources.end(), sortByElementOffset);

	Layout = RHI::RHICreateConstantBufferLayout(initializer);
}
}
