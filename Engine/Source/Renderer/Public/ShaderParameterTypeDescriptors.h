#pragma once
#include "CoreDefinitions.h"
#include "RHIDefinitions.h"
#include "Math/LinearColor.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

#include "CoreMinimum.h"
#include "Shader.h"
#include "ShaderParameterMetadata.h"
#include "RHIResources.h"
#include "DynamicRHI.h"
#include "RenderCommandList.h"

namespace LE::Renderer
{
template <typename T, int32 Alignment>
class AlignedTypedef;

template <typename PtrType>
using AlignedShaderParameterPointer = typename AlignedTypedef<PtrType, SHADER_PARAMETER_POINTER_ALIGNMENT>::Type;

#define DEFINE_ALIGNED_TYPE(Alignment) \
	template<typename T> \
	class AlignedTypedef<T, Alignment> \
	{ \
	public: \
	typedef __declspec(align(Alignment)) T Type; \
	};

DEFINE_ALIGNED_TYPE(1);

DEFINE_ALIGNED_TYPE(2);

DEFINE_ALIGNED_TYPE(4);

DEFINE_ALIGNED_TYPE(8);

DEFINE_ALIGNED_TYPE(16);
#undef DEFINE_ALIGNED_TYPE

template <typename ParameterType>
struct ShaderParameterTypeDescriptor
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Invalid;
	static constexpr int32 Alignment = sizeof(ParameterType);

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 1;
	static constexpr int32 ElementNum = 0;

	using AlignedType = ParameterType;
};

template <>
struct ShaderParameterTypeDescriptor<int32>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Int32;
	static constexpr int32 Alignment = 4;

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 1;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<int32, Alignment>::Type;
};

template <>
struct ShaderParameterTypeDescriptor<uint32>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Uint32;
	static constexpr int32 Alignment = 4;

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 1;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<uint32, Alignment>::Type;
};

template <>
struct ShaderParameterTypeDescriptor<float>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Float32;
	static constexpr int32 Alignment = 4;

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 1;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<float, Alignment>::Type;
};

template <>
struct ShaderParameterTypeDescriptor<Vector2F>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Float32;
	static constexpr int32 Alignment = 8;

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 2;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<Vector2F, Alignment>::Type;
};

template <>
struct ShaderParameterTypeDescriptor<Vector3F>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Float32;
	static constexpr int32 Alignment = 16;

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 3;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<Vector3F, Alignment>::Type;
};

template <>
struct ShaderParameterTypeDescriptor<Vector4F>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Float32;
	static constexpr int32 Alignment = 16;

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 4;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<Vector4F, Alignment>::Type;
};

template <>
struct ShaderParameterTypeDescriptor<LinearColor>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Float32;
	static constexpr int32 Alignment = 16;

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 4;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<LinearColor, Alignment>::Type;
};

template <>
struct ShaderParameterTypeDescriptor<QuaternionF>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Float32;
	static constexpr int32 Alignment = 16;

	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 4;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<QuaternionF, Alignment>::Type;
};

template <>
struct ShaderParameterTypeDescriptor<Matrix4x4F>
{
	static constexpr RHI::ShaderParameterType ResourceType = RHI::SPT_Float32;
	static constexpr int32 Alignment = 16;

	static constexpr int32 RowsNum = 4;
	static constexpr int32 ColumnNum = 4;
	static constexpr int32 ElementNum = 0;

	using AlignedType = AlignedTypedef<Matrix4x4F, Alignment>::Type;
};

template <typename ShaderResource>
struct ShaderResourceDescriptor
{
	static constexpr int32 RowsNum = 1;
	static constexpr int32 ColumnNum = 1;
	static constexpr int32 ElementNum = 1;
	static constexpr int32 Alignment = SHADER_PARAMETER_POINTER_ALIGNMENT;

	using AlignedType = AlignedShaderParameterPointer<ShaderResource>;
};

class GlobalConstantBufferRegistration
{
public:
	GlobalConstantBufferRegistration(FunctionRef<const ShaderParametersMetadata*()> ShaderParametersMetadataAccessor)
		: ShaderParametersMetadataAccessor(ShaderParametersMetadataAccessor)
	{
		GetInstances().emplace_back(this);
	}

	static Array<const GlobalConstantBufferRegistration*>& GetInstances();

private:
	FunctionRef<const ShaderParametersMetadata*()> ShaderParametersMetadataAccessor;
};

template <typename BufferStruct>
class ConstantBufferRef : public RefCountingPtr<RHI::RHIConstantBuffer>
{
public:
	ConstantBufferRef()
		: RefCountingPtr<LE::RHI::RHIConstantBuffer>(nullptr)
	{
	}

	static ConstantBufferRef<BufferStruct> CreateConstantBuffer(const BufferStruct& Value)
	{
		return ConstantBufferRef<BufferStruct>(RHI::RHICreateConstantBuffer(&Value, BufferStruct::GetParametersMetadata()->GetLayout()));
	}

	void UpdateConstantBuffer(RenderCommandList& CmdList, const BufferStruct& Value)
	{
		CmdList.UpdateConstantBuffer(GetPointer(), &Value);
	}

private:
	ConstantBufferRef(RHI::RHIConstantBuffer* InBuffer)
		: RefCountingPtr<LE::RHI::RHIConstantBuffer>(InBuffer)
	{
	}
};

#define DECLARE_SHADER(ShaderName) \
	public: \
	using Shader::Shader;	\
	static ShaderMetaType& StaticGetMetaType(); \
	static Shader* CreateCompiledShader(const CompiledShaderInitializer& Initializer) \
	{ \
		return new ShaderName(Initializer); \
	} \
	private: \
	static ShaderMetaTypeRegistration MetaTypeRegistration; \
	public:

// At least one buffer should be defined with this macro, and all the rest should be referenced here 
#define BEGIN_ROOT_CONSTANT_BUFFER(BufferName, ShaderName) \
	static inline const ShaderParametersMetadata* GetRootParametersMetadata() { return BufferName::Descriptor::GetParametersMetadata(); } \
	\
	ShaderName(const CompiledShaderInitializer& Initializer) \
		: Shader(Initializer) \
	{ \
	this->Bindings.BindRootShaderConstantBuffer(this, Initializer.ParameterAllocationsMap); \
	} \
	ShaderName() \
	{} \
	BEGIN_CONSTANT_BUFFER(BufferName)

#define DESCRIPTOR_CONTENT_SHADER_PARAMETERS(BufferName) \
	static const ShaderParametersMetadata* GetParametersMetadata()  \
			{ \
				static ShaderParametersMetadata Metadata( \
					ShaderParametersMetadata::Type::ShaderParameters, \
					#BufferName, \
					FileName, \
					sizeof(BufferName), \
					BufferName::GetParameters()); \
					return &Metadata; \
			}

#define DESCRIPTOR_CONTENT_GLOBAL_PARAMETERS(BufferName) \
	static const ShaderParametersMetadata* GetParametersMetadata()  \
			{ \
				return BufferName::GetParametersMetadata(); \
			}

#define BEGIN_CONSTANT_BUFFER_INTERNAL(BufferName, DescriptorContent) \
	struct Descriptor \
		{ \
			static constexpr int32 RowsNum = 1; \
			static constexpr int32 ColumnsNum = 1; \
			static constexpr int32 ElementsNum = 1;	\
			static constexpr int32 Alignment = SHADER_PARAMETER_ALIGNMENT; \
			static constexpr const char* FileName = __builtin_FILE(); \
			DescriptorContent \
		}; \
	static RefCountingPtr<RHI::RHIConstantBuffer> CreateRHIConstantBuffer(const BufferName& InData) \
	{ \
		return RHI::RHICreateConstantBuffer(&InData, Descriptor::GetParametersMetadata()->GetLayout()); \
	} \
	private: \
	struct FinishingParameterId\
	{ \
	}; \
	using FuncPointer = void*; \
	using BuildParameterMetadataFunction = FuncPointer(*)(FinishingParameterId, Array<ShaderParametersMetadata::Parameter>*); \
	static FuncPointer AppendParameterMetadataAndGetPrevious(FinishingParameterId, Array<ShaderParametersMetadata::Parameter>*) \
	{ \
		return nullptr; \
	} \
	using StructAlias = BufferName;	\
	typedef FinishingParameterId


#define BEGIN_CONSTANT_BUFFER(BufferName) \
	public: \
	__declspec(align(SHADER_PARAMETER_ALIGNMENT)) class BufferName \
	{ \
	public: \
	BEGIN_CONSTANT_BUFFER_INTERNAL(BufferName, DESCRIPTOR_CONTENT_SHADER_PARAMETERS(BufferName))


#define BEGIN_GLOBAL_CONSTANT_BUFFER(BufferName) \
	class BufferName; \
	const ShaderParametersMetadata* GetForwardDeclaredShaderMetadataParameters(const BufferName* Ptr); \
	__declspec(align(SHADER_PARAMETER_ALIGNMENT)) class BufferName \
	{ \
	public: \
		  BufferName() \
		  {} \
		  static const ShaderParametersMetadata* GetParametersMetadata(); \
		  BEGIN_CONSTANT_BUFFER_INTERNAL(BufferName, DESCRIPTOR_CONTENT_GLOBAL_PARAMETERS(BufferName))

#define IMPLEMENT_GLOBAL_CONSTANT_BUFFER(BufferName, VariableName) \
	const ShaderParametersMetadata* GetForwardDeclaredShaderMetadataParameters(const BufferName* Ptr) \
	{ \
		return BufferName::Descriptor::GetParametersMetadata(); \
	} \
	const ShaderParametersMetadata* BufferName::GetParametersMetadata() \
	{ \
		static ShaderParametersMetadata Metadata( \
					ShaderParametersMetadata::Type::GlobalBuffer, \
					VariableName, \
					__builtin_FILE(), \
					sizeof(BufferName), \
					BufferName::GetParameters()); \
		return &Metadata; \
	} \
	GlobalConstantBufferRegistration BufferName##Registration { FunctionRef<const ShaderParametersMetadata*()>{	BufferName::GetParametersMetadata } };

#define SHADER_TYPE_DESCRIPTOR(ParameterType) ShaderParameterTypeDescriptor<ParameterType>

#define SHADER_PARAMETER_OFFSET(ParameterName) static_cast<uint32>(reinterpret_cast<std::uintptr_t>(&(reinterpret_cast<char const volatile&>(static_cast<StructAlias*>(nullptr)->ParameterName))))

#define SHADER_PARAMETER_INTERNAL(ParameterType, ParameterName, Descriptor) \
	ParameterName##Id; \
	public: \
		Descriptor::AlignedType ParameterName; \
		static_assert(Descriptor::ResourceType != RHI::SPT_Invalid); \
	private: \
		struct ParameterIdAfter##ParameterName {}; \
		static FuncPointer AppendParameterMetadataAndGetPrevious(ParameterIdAfter##ParameterName, Array<ShaderParametersMetadata::Parameter>* Parameters) \
		{ \
			Parameters->emplace_back(ShaderParametersMetadata::Parameter( \
				#ParameterName, \
				SHADER_PARAMETER_OFFSET(ParameterName), \
				Descriptor::ResourceType, \
				Descriptor::RowsNum, \
				Descriptor::ColumnNum, \
				Descriptor::ElementNum)); \
			using PreviousFunction = FuncPointer(*)(ParameterName##Id, Array<ShaderParametersMetadata::Parameter>*); \
			PreviousFunction Prev = AppendParameterMetadataAndGetPrevious; \
			return static_cast<FuncPointer>(Prev); \
		} \
		typedef ParameterIdAfter##ParameterName


#define DECLARE_SHADER_PARAMETER(ParameterType, ParameterName) \
	SHADER_PARAMETER_INTERNAL(ParameterType, ParameterName, SHADER_TYPE_DESCRIPTOR(ParameterType))

#define INCLUDE_CONSTANT_BUFFER(ConstantBuffer, ParameterName) \
	ParameterName##Id; \
	public: \
		ConstantBuffer ParameterName; \
	private: \
		struct ParameterIdAfter##ParameterName {}; \
		static FuncPointer AppendParameterMetadataAndGetPrevious(ParameterIdAfter##ParameterName, Array<ShaderParametersMetadata::Parameter>* Parameters) \
		{ \
			const auto& parameters = ConstantBuffer::Descriptor::GetParametersMetadata()->GetParameters(); \
			uint32 baseOffset = SHADER_PARAMETER_OFFSET(ParameterName); \
			for(const auto& it : parameters) \
			{ \
				Parameters->emplace_back(ShaderParametersMetadata::Parameter( \
					it.GetName(), \
					baseOffset + it.GetOffset(), \
					it.GetShaderParameterType(), \
					it.GetRowsNum(), \
					it.GetColumnsNum(), \
					it.GetElementsNum())); \
			} \
			using PreviousFunction = FuncPointer(*)(ParameterName##Id, Array<ShaderParametersMetadata::Parameter>*); \
			PreviousFunction Prev = AppendParameterMetadataAndGetPrevious; \
			return static_cast<FuncPointer>(Prev); \
		} \
		typedef ParameterIdAfter##ParameterName

#define INTERNAL_SHADER_RESOURCE_DECLARATION(ParameterType, Descriptor, BaseType, ParameterName) \
   ParameterName##Id; \
	public: \
		Descriptor::AlignedType ParameterName; \
		static_assert(RHI::ParameterType != RHI::SPT_Invalid); \
	private: \
		struct ParameterIdAfter##ParameterName {}; \
		static FuncPointer AppendParameterMetadataAndGetPrevious(ParameterIdAfter##ParameterName, Array<ShaderParametersMetadata::Parameter>* Parameters) \
		{ \
			Parameters->emplace_back(ShaderParametersMetadata::Parameter( \
				#ParameterName, \
				SHADER_PARAMETER_OFFSET(ParameterName), \
				RHI::ParameterType, \
				Descriptor::RowsNum, \
				Descriptor::ColumnNum, \
				Descriptor::ElementNum)); \
			using PreviousFunction = FuncPointer(*)(ParameterName##Id, Array<ShaderParametersMetadata::Parameter>*); \
			PreviousFunction Prev = AppendParameterMetadataAndGetPrevious; \
			return static_cast<FuncPointer>(Prev); \
		} \
		typedef ParameterIdAfter##ParameterName

#define DECLARE_SHADER_PARAMETER_SAMPLER(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Sampler, ShaderResourceDescriptor<RHI::RHISamplerState*>, RGTexture*, ParameterName)

#define DECLARE_GLOBAL_SHADER_PARAMETER_TEXTURE(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Global_Texture, ShaderResourceDescriptor<RHI::RHITexture*>, RHI::RHITexture*, ParameterName)

#define DECLARE_GLOBAL_SHADER_PARAMETER_BUFFER(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Global_Buffer, ShaderResourceDescriptor<RHI::RHIBuffer*>, RHI::RHIBuffer*, ParameterName)

#define DECLARE_GLOBAL_SHADER_PARAMETER_READ_VIEW(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Global_ReadView, ShaderResourceDescriptor<RHI::RHIReadView*>, RHI::RHIReadView*, ParameterName)

#define DECLARE_GLOBAL_SHADER_PARAMETER_WRITE_VIEW(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Global_WriteView, ShaderResourceDescriptor<RHI::RHIWriteView*>, RHI::RHIWriteView*, ParameterName)

#define DECLARE_SHADER_PARAMETER_TEXTURE(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Texture, ShaderResourceDescriptor<RGTexture*>, RGTexture*, ParameterName)

#define DECLARE_SHADER_PARAMETER_TEXTURE_READ_VIEW(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Texture_ReadView, ShaderResourceDescriptor<RGTextureReadView*>, RGTextureReadView*, ParameterName)

#define DECLARE_SHADER_PARAMETER_TEXTURE_WRITE_VIEW(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Texture_WriteView, ShaderResourceDescriptor<RGTextureWriteView*>, RGTextureWriteView*, ParameterName)

#define DECLARE_SHADER_PARAMETER_BUFFER_READ_VIEW(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Buffer_ReadView, ShaderResourceDescriptor<RGBufferReadView*>, RGBufferReadView*, ParameterName)

#define DECLARE_SHADER_PARAMETER_BUFFER_WRITE_VIEW(ParameterName) \
	INTERNAL_SHADER_RESOURCE_DECLARATION(SPT_Buffer_WriteView, ShaderResourceDescriptor<RGBufferWriteView*>, RGBufferWriteView*, ParameterName)

#define END_CONSTANT_BUFFER() \
	FirstParameterId; \
	public: \
		static Array<ShaderParametersMetadata::Parameter> GetParameters() \
		{ \
			Array<ShaderParametersMetadata::Parameter> Parameters; \
			using FirstFunction = FuncPointer(*)(FirstParameterId, Array<ShaderParametersMetadata::Parameter>*); \
			FirstFunction First = AppendParameterMetadataAndGetPrevious; \
			FuncPointer Pointer = static_cast<FuncPointer>(First); \
			do \
			{ \
				Pointer = reinterpret_cast<BuildParameterMetadataFunction>(Pointer)(FinishingParameterId(), &Parameters); \
			} while (Pointer); \
			std::reverse(Parameters.begin(), Parameters.end()); \
			return Parameters; \
		} \
	};
}
