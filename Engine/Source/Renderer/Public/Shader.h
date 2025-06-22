#pragma once
#include "RenderResource.h"
#include "ShaderCompilerCore.h"
#include "ShaderCore.h"
#include "ShaderParameterMetadata.h"
#include "ShaderParameters.h"
#include "Containers/Array.h"
#include "Templates/RefCounters.h"


namespace LE::Renderer
{
class MeshConverterType;
class ShaderParametersMapInfo;
class Shader;
}

namespace LE::Renderer
{
class ShaderMetaType;
class ShaderParametersMetadata;
struct CompiledShaderInitializer;

class ShaderParameterBindings
{
public:
	// Represent resource parameter (texture, sampler etc)
	struct ResourceParameter
	{
		uint32 ByteOffset;
		uint32 BaseIndex;
		RHI::ShaderParameterType Type;
	};

	struct ComplexParameter
	{
		uint32 ByteOffset;
		uint32 BufferIndex;
	};

	static constexpr uint32 InvalidBufferIndex = 0xFFFFF;

	void BindRootShaderConstantBuffer(const Shader* Shader, const ShaderParameterAllocationMap& ParametersMap);

	Array<ResourceParameter> ResourceParameters;
	Array<ComplexParameter> ConstantBuffers;

	uint32 RootConstantBufferIndex = InvalidBufferIndex;
};

class ShaderConstantBufferParameterInfo
{
public:
	ShaderConstantBufferParameterInfo() = default;

	ShaderConstantBufferParameterInfo(uint32 InBaseIndex)
		: BaseIndex(InBaseIndex)
	{
	}

	bool operator==(const ShaderConstantBufferParameterInfo& Other) const
	{
		return BaseIndex == Other.BaseIndex;
	}

	bool operator<(const ShaderConstantBufferParameterInfo& Other) const
	{
		return BaseIndex < Other.BaseIndex;
	}

	uint32 BaseIndex;
};

class ShaderResourceParameterInfo
{
public:
	ShaderResourceParameterInfo() = default;

	ShaderResourceParameterInfo(uint32 InBaseIndex, uint32 InBufferIndex, ShaderReflectedParameterType InType)
		: BaseIndex(InBaseIndex)
		  , BufferIndex(InBufferIndex)
		  , Type(InType)
	{
	}

	bool operator==(const ShaderResourceParameterInfo& Other) const
	{
		return BaseIndex == Other.BaseIndex && BufferIndex == Other.BufferIndex && Type == Other.Type;
	}

	bool operator<(const ShaderResourceParameterInfo& Other) const
	{
		return BaseIndex < Other.BaseIndex;
	}

	uint32 BaseIndex;
	uint32 BufferIndex;
	ShaderReflectedParameterType Type;
};

class ShaderParametersMapInfo
{
public:
	Array<ShaderConstantBufferParameterInfo> ConstantBuffers;
	Array<ShaderResourceParameterInfo> TextureSamplers;
	Array<ShaderResourceParameterInfo> ReadOnlyViews;
};

class Shader : public RefCountableBase
{
public:
	Shader() = default;
	Shader(const CompiledShaderInitializer& Initializer);

	const ShaderMetaType* GetMetaType() const { return MetaType; }
	uint32 GetCodeSize() const { return CodeSize; }
	RHI::ShaderType GetShaderType() const;

	template<typename ConstantBufferType>
	bool HasConstantBufferParameter() const
	{
		return HasConstantBufferParameter(ConstantBufferType::GetParametersMetadata());
	}

	bool HasConstantBufferParameter(const ShaderParametersMetadata* Struct) const;
	bool HasConstantBufferParameter(const char* ConstantBufferName) const;

	template<typename ConstantBufferType>
	const ShaderConstantBufferParameter& GetConstantBufferParameter() const
	{
		return GetConstantBufferParameter(ConstantBufferType::GetParametersMetadata());
	}

	const ShaderConstantBufferParameter& GetConstantBufferParameter(const ShaderParametersMetadata* Struct) const;
	const ShaderConstantBufferParameter& GetConstantBufferParameter(const char* ConstantBufferName) const;

	const ShaderResourceParameter& GetResourceParameter(const ShaderParametersMetadata::Parameter* Parameter) const;
	const ShaderResourceParameter& GetResourceParameter(const char* ResourceName) const;

	// All other parameters are referenced there
	static inline const ShaderParametersMetadata* GetRootParametersMetadata()
	{
		return nullptr;
	}

private:
	void TryBindingConstantBuffer(const ShaderParameterAllocationMap& AllocationMap, const ShaderParametersMetadata* ConstantBufferMetadata);


public:
	ShaderParameterBindings Bindings;
	ShaderParametersMapInfo ReflectionParametersMapInfo;

protected:
	Array<String> ConstantBufferNames;
	Array<String> ResourceParameterNames;
	Array<ShaderConstantBufferParameter> ConstantBufferParameters;
	Array<ShaderResourceParameter> ResourceParameters;

private:
	void BuildParameterMapInfo(const Map<String, ShaderParameterAllocation>& ParameterMap);

	const ShaderMetaType* MetaType;
	const MeshConverterType* MConverterType;
	uint32 CodeSize;
};

class ShaderMetaType
{
public:
	class Parameters
	{
	public:
		virtual ~Parameters()
		{
		}
	};

	typedef Shader* (*CreateCompiledType)(const CompiledShaderInitializer& Initializer);

	static LinkedList<ShaderMetaType*>*& GetTypeList();
	static ShaderMetaType* GetShaderMetaTypeByName(const char* Name);
	static Map<String, ShaderMetaType*>& GetNameToMetaTypeMap();

	ShaderMetaType(const char* InName, const char* InSourceFileName, const char* InMainFunctionName, RHI::ShaderType InShaderType,
	               CreateCompiledType InCreateCompiledRef,
	               const ShaderParametersMetadata* InParameterMetadata);
	~ShaderMetaType();

	Shader* CreateCompiled(const CompiledShaderInitializer& Initializer) const;

	RHI::ShaderType GetShaderType() const { return ShaderType; }
	const char* GetName() const { return Name; }
	const char* GetSourceFileName() const { return SourceFileName; }
	const char* GetMainFunctionName() const { return MainFunctionName; }
	const ShaderParametersMetadata* GetShaderParameterMetadata() const { return ParametersMetadata; }

protected:
	const char* Name;
	const char* SourceFileName;
	const char* MainFunctionName;
	RHI::ShaderType ShaderType;
	const ShaderParametersMetadata* ParametersMetadata;

	CreateCompiledType CreateCompiledRef;
};

struct CompiledShaderInitializer
{
	CompiledShaderInitializer(const ShaderMetaType* InShaderMetaType,
	                          const ShaderCompilerResult& InCompilerResult,
	                          const MeshConverterType* InMeshConverterType);
	const ShaderMetaType* MetaType;
	const Array<uint8>& Code;
	const ShaderParameterAllocationMap& ParameterAllocationsMap;
	const MeshConverterType* ConverterType;
	uint32 CodeSize;
};

class ShaderMetaTypeRegistration
{
public:
	ShaderMetaTypeRegistration(FunctionRef<ShaderMetaType&()> InShaderAccessor)
		: ShaderAccessor(std::move(InShaderAccessor))
	{
		GetInstances().push_back(this);
	}

	static void RegisterAll();

	static Array<const ShaderMetaTypeRegistration*>& GetInstances();

private:
	FunctionRef<ShaderMetaType&()> ShaderAccessor;
};

inline RHI::RHIResource* GetShaderParameterResourceRHI(const void* Data, uint32 MemberOffset, RHI::ShaderParameterType ParameterType)
{
	if (ParameterType == RHI::SPT_Constant_Buffer)
	{
		return nullptr;
	}

	const uint8* parameterPtr = reinterpret_cast<const uint8*>(Data) + MemberOffset;
	RHI::RHIResource* resourcePtr = *reinterpret_cast<RHI::RHIResource* const *>(parameterPtr);
	return resourcePtr;
}

#define REGISTER_SHADER(ShaderClass, SourceFilePath, MainFunction, RHIShaderType) \
	inline ShaderMetaType& ShaderClass::StaticGetMetaType()	\
	{ \
		static ShaderMetaType StaticMetaType( \
			#ShaderClass, \
			SourceFilePath, \
			MainFunction, \
			RHIShaderType, \
			ShaderClass::CreateCompiledShader, \
			ShaderClass::GetRootParametersMetadata()); \
		return StaticMetaType; \
	} \
	ShaderMetaTypeRegistration ShaderClass::MetaTypeRegistration{FunctionRef<ShaderMetaType&()>{ShaderClass::StaticGetMetaType}};
}
