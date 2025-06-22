#pragma once
#include <cstdint>

#include "CoreDefinitions.h"
#include "Misc/EnumFlags.h"

#define SHADER_PARAMETER_ALIGNMENT 16
#define SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT 16
#define SHADER_PARAMETER_POINTER_ALIGNMENT sizeof(uint64)

#define MAX_SIMULTANEOUS_RENDER_TARGETS 8

namespace LE::RHI
{
enum class RHIInterfaceType : uint8
{
	None,

	D3D11,

	Count,
};

enum class RHIResourceType : uint8
{
	None,

	Buffer,
	Texture,
	Viewport,
	VertexShader,
	PixelShader,
	DepthStencilState,
	ConstantBufferLayout,
	ConstantBuffer,
	VertexBufferLayout,
	SamplerState,
	ReadView,
	WriteView,
	PipelineStateObject,
	BoundShaderState,

	Count,
};

enum class ShaderType : uint8
{
	Vertex = 0,
	Pixel = 1,

	Start = 0,
	Count = 2,
};

enum BufferUsageFlags : uint8
{
	BUF_None = 0,

	BUF_Vertex = 1 << 0,
	BUF_Index = 1 << 1,
	BUF_ShaderResource = 1 << 2,
};

ENUM_CLASS_FLAGS(BufferUsageFlags)

enum TextureCreateFlags: uint8
{
	TCF_None = 0,

	TCF_RenderTargetable = 1 << 0,
	TCF_DepthStencilTargetable = 1 << 1,
	TCF_CPUReadBack = 1 << 2,
	TCF_CPUWrite = 1 << 3,
	TCF_ShaderResource = 1 << 4,
};

ENUM_CLASS_FLAGS(TextureCreateFlags)

enum class TextureDimensions: uint8
{
	Texture2D,
	Texture3D,
};

enum class CompareFunction : uint8
{
	Less,
	LessEqual,
	Greater,
	GreaterEqual,
	Equal,
	NotEqual,
	Never,
	Always,

	Count,
};

enum class StencilOp : uint8
{
	Keep,
	Zero,
	Replace,
	SaturatedIncrement,
	SaturatedDecrement,
	Invert,
	Increment,
	Decrement,

	Count,
};

enum class SamplerFilter : uint8
{
	Point,
	Bilinear,
	Trilinear,
	Anisotropic,

	Count,
};

enum class SamplerAddressMode : uint8
{
	Wrap,
	Clamp,
	Mirror,
	Border,

	Count,
};

enum class SamplerCompareFunction : uint8
{
	Never,
	Less
};

enum ShaderParameterType : uint8
{
	SPT_Invalid,

	SPT_Int32,
	SPT_Uint32,
	SPT_Float32,

	SPT_Sampler,

	SPT_Global_Buffer,
	SPT_Global_Texture,

	SPT_Global_ReadView,
	SPT_Global_WriteView,

	SPT_Texture,
	SPT_Texture_ReadView,
	SPT_Texture_WriteView,

	SPT_Buffer_ReadView,
	SPT_Buffer_WriteView,

	SPT_Constant_Buffer,

	SPT_Included_CBuffer,

	SPT_Count
};

enum class VertexElementType : uint8
{
	Invalid,

	Float2,
	Float3,

	Count,
};

enum class RenderTargetLoadAction : uint8
{
	NoAction,
	Load,
	Clear,

	Count,
};

enum class RenderTargetStoreAction : uint8
{
	NoAction,
	Store,

	Count,
};

enum class PrimitiveType : uint8
{
	TriangleList,
	TriangleStrip,
	LineList,
	PointList,

	Count
};

enum class PixelFormat : uint8
{
	Invalid,

	R8G8B8A8_UNORM,
	R32G32B32A32_FLOAT,
	R32G32_FLOAT,

	Count
};

struct PixelFormatInfo
{
	PixelFormatInfo() = delete;

	PixelFormatInfo(PixelFormat InFormat, uint32 InBlockBytes)
		: Format(InFormat)
		  , BlockBytes(InBlockBytes)
	{
	}

	PixelFormat Format;
	uint32 BlockBytes;
};

extern PixelFormatInfo GPixelFormats[static_cast<uint8>(PixelFormat::Count)];
}
