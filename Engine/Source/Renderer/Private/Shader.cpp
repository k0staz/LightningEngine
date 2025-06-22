#include "Shader.h"

#include "ShaderParameterMetadata.h"
#include "ShaderParameters.h"
#include "MeshConverters/MeshConverter.h"
#include "MaterialShaderAutoRegistration.h"
#include "ShaderParameterMetadata.h"

namespace LE::Renderer
{
static LinkedList<ShaderMetaType*>* GShaderTypeLists = nullptr;
static Array<const ShaderMetaTypeRegistration*>* gShaderMetaTypeRegistrations = nullptr;

void ShaderParameterBindings::BindRootShaderConstantBuffer(const Shader* Shader, const ShaderParameterAllocationMap& ParametersMap)
{
	const ShaderMetaType* metaType = Shader->GetMetaType();
	LE_ASSERT(this == &Shader->Bindings)
	LE_ASSERT(metaType->GetShaderParameterMetadata())

	const ShaderParametersMetadata& rootParameters = *metaType->GetShaderParameterMetadata();

	for (const ShaderParametersMetadata::Parameter& rootParameter : rootParameters.GetParameters())
	{
		if (rootParameter.IsParameterNativeType())
			continue;

		RHI::ShaderParameterType parameterType = rootParameter.GetShaderParameterType();
		const uint32 elementNumber = rootParameter.GetElementsNum();
		const bool isArray = elementNumber > 0;
		for (uint32 elementIdx = 0; elementIdx < (isArray ? elementNumber : 1u); ++elementIdx)
		{
			Optional<ShaderParameterAllocation> parameterAllocation = ParametersMap.GetParameterAllocation(rootParameter.GetName());
			if (!parameterAllocation.has_value())
			{
				continue;
			}

			if (parameterType == RHI::SPT_Constant_Buffer)
			{
				LE_ASSERT(!isArray)
				ConstantBuffers.emplace_back(rootParameter.GetOffset(), parameterAllocation->BufferIndex);
			}
			else
			{
				ResourceParameters.emplace_back(rootParameter.GetOffset() + elementIdx * static_cast<uint32>(SHADER_PARAMETER_POINTER_ALIGNMENT),
				                                parameterAllocation->BaseIndex, parameterType);
			}
		}
	}

	Optional<ShaderParameterAllocation> rootParameterAllocation = ParametersMap.GetParameterAllocation(
		rootParameters.GetConstantBufferName());
	if (rootParameterAllocation.has_value())
	{
		RootConstantBufferIndex = rootParameterAllocation->BufferIndex;
	}
}

Shader::Shader(const CompiledShaderInitializer& Initializer)
	: MetaType(Initializer.MetaType)
	  , MConverterType(Initializer.ConverterType)
	  , CodeSize(Initializer.CodeSize)

{
	BuildParameterMapInfo(Initializer.ParameterAllocationsMap.GetParametersMap());

	// Bind local constant buffers
	TryBindingConstantBuffer(Initializer.ParameterAllocationsMap, MetaType->GetShaderParameterMetadata());

	// Bind global constant buffers
	for (const ShaderParametersMetadata* globalConstBuffer : *ShaderParametersMetadata::GetParametersList())
	{
		TryBindingConstantBuffer(Initializer.ParameterAllocationsMap, globalConstBuffer);
	}
}

RHI::ShaderType Shader::GetShaderType() const
{
	return MetaType->GetShaderType();
}

bool Shader::HasConstantBufferParameter(const ShaderParametersMetadata* Struct) const
{
	return HasConstantBufferParameter(Struct->GetConstantBufferName());
}

bool Shader::HasConstantBufferParameter(const char* ConstantBufferName) const
{
	for (uint32 index = 0; index < ConstantBufferNames.Count(); ++index)
	{
		if (ConstantBufferNames[index] == ConstantBufferName)
		{
			return true;
		}
	}

	return false;
}

const ShaderConstantBufferParameter& Shader::GetConstantBufferParameter(const ShaderParametersMetadata* Struct) const
{
	return GetConstantBufferParameter(Struct->GetConstantBufferName());
}

const ShaderConstantBufferParameter& Shader::GetConstantBufferParameter(const char* ConstantBufferName) const
{
	uint32 bufferIndex = 0;
	bool wasFound = false;
	for (uint32 index = 0; index < ConstantBufferNames.Count(); ++index)
	{
		if (ConstantBufferNames[index] == ConstantBufferName)
		{
			bufferIndex = index;
			wasFound = true;
			break;
		}
	}

	if (wasFound)
	{
		return ConstantBufferParameters[bufferIndex];
	}
	else
	{
		static ShaderConstantBufferParameter unboundParameter;
		return unboundParameter;
	}
}

const ShaderResourceParameter& Shader::GetResourceParameter(const ShaderParametersMetadata::Parameter* Parameter) const
{
	return GetResourceParameter(Parameter->GetName());
}

const ShaderResourceParameter& Shader::GetResourceParameter(const char* ResourceName) const
{
	uint32 resourceIndex = 0;
	bool wasFound = false;
	for (uint32 index = 0; index < ResourceParameterNames.Count(); ++index)
	{
		if (ResourceParameterNames[index] == ResourceName)
		{
			resourceIndex = index;
			wasFound = true;
			break;
		}
	}

	if (wasFound)
	{
		return ResourceParameters[resourceIndex];
	}
	else
	{
		static ShaderResourceParameter unboundParameter;
		return unboundParameter;
	}
}

void Shader::TryBindingConstantBuffer(const ShaderParameterAllocationMap& AllocationMap, const ShaderParametersMetadata* ConstantBufferMetadata)
{
	if (AllocationMap.HasParameterAllocation(ConstantBufferMetadata->GetConstantBufferName()))
	{
		ConstantBufferNames.emplace_back(ConstantBufferMetadata->GetConstantBufferName());
		ShaderConstantBufferParameter& parameter = ConstantBufferParameters.emplace_back();
		parameter.Bind(AllocationMap, ConstantBufferMetadata->GetConstantBufferName());
	}

	for (const ShaderParametersMetadata::Parameter& parameter : ConstantBufferMetadata->GetParameters())
	{
		if (parameter.IsParameterNativeType())
		{
			continue;
		}

		if (AllocationMap.HasParameterAllocation(parameter.GetName()))
		{
			ResourceParameterNames.emplace_back(parameter.GetName());
			ShaderResourceParameter& resource = ResourceParameters.emplace_back();
			resource.Bind(AllocationMap, parameter.GetName());
		}
	}
}

void Shader::BuildParameterMapInfo(const Map<String, ShaderParameterAllocation>& ParameterMap)
{
	uint32 samplerCount = 0;
	uint32 constantBufferCount = 0;
	uint32 readOnlyViewCount = 0;

	for (auto& parameter : ParameterMap)
	{
		const ShaderParameterAllocation& parameterAllocation = parameter.second;

		switch (parameterAllocation.Type)
		{
		case ShaderReflectedParameterType::Sampler:
			++samplerCount;
			break;
		case ShaderReflectedParameterType::ConstantBuffer:
			++constantBufferCount;
		case ShaderReflectedParameterType::ReadOnlyView:
			++readOnlyViewCount;
		default:
			break;
		}
	}

	ReflectionParametersMapInfo.TextureSamplers.reserve(samplerCount);
	ReflectionParametersMapInfo.ConstantBuffers.reserve(constantBufferCount);
	ReflectionParametersMapInfo.ReadOnlyViews.reserve(readOnlyViewCount);

	for (const auto& parameterIterator : ParameterMap)
	{
		const ShaderParameterAllocation& parameterAllocation = parameterIterator.second;

		switch (parameterAllocation.Type)
		{
		case ShaderReflectedParameterType::Sampler:
			ReflectionParametersMapInfo.TextureSamplers.emplace_back(parameterAllocation.BaseIndex, parameterAllocation.BufferIndex,
			                                                         parameterAllocation.Type);
			break;
		case ShaderReflectedParameterType::ConstantBuffer:
			ReflectionParametersMapInfo.ConstantBuffers.emplace_back(parameterAllocation.BufferIndex);
			break;
		case ShaderReflectedParameterType::ReadOnlyView:
			ReflectionParametersMapInfo.ReadOnlyViews.emplace_back(parameterAllocation.BaseIndex, parameterAllocation.BufferIndex,
			                                                       parameterAllocation.Type);
			break;
		default:
			break;
		}
	}

	std::sort(ReflectionParametersMapInfo.TextureSamplers.begin(), ReflectionParametersMapInfo.TextureSamplers.end());
	std::sort(ReflectionParametersMapInfo.ConstantBuffers.begin(), ReflectionParametersMapInfo.ConstantBuffers.end());
	std::sort(ReflectionParametersMapInfo.ReadOnlyViews.begin(), ReflectionParametersMapInfo.ReadOnlyViews.end());
}

LinkedList<ShaderMetaType*>*& ShaderMetaType::GetTypeList()
{
	if (!GShaderTypeLists)
	{
		GShaderTypeLists = new LinkedList<ShaderMetaType*>();
	}
	return GShaderTypeLists;
}

ShaderMetaType* ShaderMetaType::GetShaderMetaTypeByName(const char* Name)
{
	const auto& iterator = GetNameToMetaTypeMap().find(Name);
	if (iterator != GetNameToMetaTypeMap().end())
	{
		return iterator->second;
	}

	return nullptr;
}

Map<String, ShaderMetaType*>& ShaderMetaType::GetNameToMetaTypeMap()
{
	static Map<String, ShaderMetaType*> shaderNameToMetaTypeMap;
	return shaderNameToMetaTypeMap;
}

ShaderMetaType::ShaderMetaType(const char* InName, const char* InSourceFileName, const char* InMainFunctionName,
                               RHI::ShaderType InShaderType, CreateCompiledType InCreateCompiledRef,
                               const ShaderParametersMetadata* InParameterMetadata)
	: Name(InName)
	  , SourceFileName(InSourceFileName)
	  , MainFunctionName(InMainFunctionName)
	  , ShaderType(InShaderType)
	  , CreateCompiledRef(InCreateCompiledRef)
	  , ParametersMetadata(InParameterMetadata)
{
	GetTypeList()->push_front(this);
	GetNameToMetaTypeMap().emplace(InName, this);
}

ShaderMetaType::~ShaderMetaType()
{
	GetTypeList()->remove(this);
	GetNameToMetaTypeMap().erase(Name);
}

Shader* ShaderMetaType::CreateCompiled(const CompiledShaderInitializer& Initializer) const
{
	return (*CreateCompiledRef)(Initializer);
}

CompiledShaderInitializer::CompiledShaderInitializer(const ShaderMetaType* InShaderMetaType,
                                                     const ShaderCompilerResult& InCompilerResult,
                                                     const MeshConverterType* InMeshConverter)
	: MetaType(InShaderMetaType)
	  , Code(InCompilerResult.Code)
	  , ParameterAllocationsMap(InCompilerResult.ParametersMap)
	  , CodeSize(InCompilerResult.CodeSize)
	  , ConverterType(InMeshConverter)
{
}

void ShaderMetaTypeRegistration::RegisterAll()
{
	RegisterAllMaterialShader();

	for (auto& instance : GetInstances())
	{
		ShaderMetaType& shaderMetaType = instance->ShaderAccessor();
		LE_INFO("Shader {} is registered", shaderMetaType.GetName());
	}
	GetInstances().clear();
}

Array<const ShaderMetaTypeRegistration*>& ShaderMetaTypeRegistration::GetInstances()
{
	if (!gShaderMetaTypeRegistrations)
	{
		gShaderMetaTypeRegistrations = new Array<const ShaderMetaTypeRegistration*>();
	}
	return *gShaderMetaTypeRegistrations;
}
}
