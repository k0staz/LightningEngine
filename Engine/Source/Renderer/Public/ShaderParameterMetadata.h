#pragma once
#include "Core.h"
#include "CoreDefinitions.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "Templates/Alignment.h"
#include "Templates/RefCounters.h"

namespace LE::RHI
{
struct RHIConstantBufferInitializer;
}

namespace LE::Renderer
{
class ShaderParametersMetadata
{
public:
	static LinkedList<ShaderParametersMetadata*>*& GetParametersList();

	enum class Type : uint8
	{
		ShaderParameters,
		GlobalBuffer,
	};

	class Parameter
	{
	public:
		Parameter(const char* InName, uint32 InOffset, RHI::ShaderParameterType InResourceType, uint32 InRowsNum, uint32 InColumnsNum,
		          uint32 InElementsNum)
			: Name(InName)
			  , Offset(InOffset)
			  , Type(InResourceType)
			  , RowsNum(InRowsNum)
			  , ColumnsNum(InColumnsNum)
			  , ElementsNum(InElementsNum)
		{
		}

		const char* GetName() const { return Name; }
		uint32 GetOffset() const { return Offset; }
		RHI::ShaderParameterType GetShaderParameterType() const { return Type; }
		uint32 GetRowsNum() const { return RowsNum; }
		uint32 GetColumnsNum() const { return ColumnsNum; }
		uint32 GetElementsNum() const { return ElementsNum; }

		bool IsParameterNativeType() const
		{
			return Type == RHI::SPT_Int32 || Type == RHI::SPT_Uint32 || Type == RHI::SPT_Float32;
		}

		uint32 GetSize() const
		{
			LE_ASSERT(IsParameterNativeType())
			uint32 elementSize = sizeof(uint32) * RowsNum * ColumnsNum;
			if (ElementsNum > 0)
			{
				return Align(elementSize, SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT) * ElementsNum;
			}

			return elementSize;
		}

	private:
		const char* Name;
		uint32 Offset;
		RHI::ShaderParameterType Type;
		uint32 RowsNum;
		uint32 ColumnsNum;
		uint32 ElementsNum;
	};

	ShaderParametersMetadata(
		Type InType,
		const char* InConstantBufferName,
		const char* InFileName,
		uint32 InSize,
		const Array<Parameter>& InParameters);

	const char* GetConstantBufferName() const { return ConstantBufferName; }
	const char* GetFileName() const { return FileName; }
	uint32 GetSize() const { return Size; }
	Type GetType() const { return ParametersType; }
	RefCountingPtr<const RHI::RHIConstantBufferLayout> GetLayout() const { return Layout; }

	const Array<Parameter>& GetParameters() const { return Parameters; }
	const Parameter* GetParameter(const String& ParameterName) const;

	bool IsLayoutInitialized() const { return Layout != nullptr; }

private:
	void Initialize(RHI::RHIConstantBufferInitializer* OutBufferInitializer = nullptr);

	const char* ConstantBufferName;
	const char* FileName;
	const uint32 Size;
	const Type ParametersType;
	RefCountingPtr<const RHI::RHIConstantBufferLayout> Layout;
	Array<Parameter> Parameters;
};
}
