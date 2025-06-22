#pragma once
#include "RenderResource.h"
#include "ShaderCompilerCore.h"

namespace LE::Renderer
{
struct MeshElement;
class Shader;
class MeshDrawSingleShaderBindings;

struct VertexInputStreamSegment
{
	VertexInputStreamSegment()
		: VertexBuffer(nullptr)
		  , ElementType(RHI::VertexElementType::Invalid)
		  , StreamOffset(0)
		  , Offset(0)
		  , Stride(0)
	{
	}


	VertexInputStreamSegment(const VertexBuffer* InVertexBuffer, RHI::VertexElementType InElementType, uint32 InStreamOffset,
	                         uint32 InOffset, uint32 InStride)
		: VertexBuffer(InVertexBuffer)
		  , ElementType(InElementType)
		  , StreamOffset(InStreamOffset)
		  , Offset(InOffset)
		  , Stride(InStride)
	{
	}


	const VertexBuffer* VertexBuffer;
	RHI::VertexElementType ElementType;
	uint32 StreamOffset;
	uint32 Offset;
	uint32 Stride;
};

struct VertexInputStream
{
	VertexInputStream()
		: Offset(0)
		  , Stride(0)
		  , VertexBuffer(nullptr)
	{
	}

	VertexInputStream(uint32 InOffset, uint32 InStride, const VertexBuffer* InVertexBuffer)
		: Offset(InOffset)
		  , Stride(InStride)
		  , VertexBuffer(InVertexBuffer)
	{
	}

	uint32 Offset;
	uint32 Stride;
	const VertexBuffer* VertexBuffer;
};

class MeshConverterType
{
public:
	MeshConverterType(
		const char* InConverterName,
		const char* InAssociatedShaderFile
	);

	virtual ~MeshConverterType();

	const char* GetName() const { return Name; }

	static Array<MeshConverterType*>*& GetTypeList();
	static MeshConverterType* GetTypeByName(String& Name);

	void ModifyCompilationConfigFunction(ShaderCompilationConfig& CompilationConfig) const
	{
		CompilationConfig.IncludeStrings.push_back(std::format("#include \"%s\"", ShaderFileName));
	}

private:
	const char* Name;
	const char* ShaderFileName;
};

class MeshConverter : public RenderResource
{
public:
	MeshConverter() = default;

	virtual MeshConverterType* GetMeshConverterType() const { return nullptr; }

	virtual void GetShaderBindings(MeshDrawSingleShaderBindings& ShaderBindings, const Shader* Shader, const MeshElement& MeshElement) const = 0;

protected:
	Array<VertexInputStream> Streams;
};

#define DECLARE_MESH_CONVERTER_TYPE() \
	public: \
		static MeshConverterType StaticType; \
		virtual MeshConverterType* GetMeshConverterType() const override;

#define IMPLEMENT_MESH_CONVERTER_TYPE(ConverterClass, ShaderFilename) \
	MeshConverterType ConverterClass::StaticType( \
		#ConverterClass, \
		ShaderFilename	\
		); \
		MeshConverterType* ConverterClass::GetMeshConverterType() const { return &StaticType; }
}
