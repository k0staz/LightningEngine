#pragma once
#include <cstdint>

#include "CoreDefinitions.h"
#include "Misc/EnumFlags.h"

#define EnableBitwiseOperations(EnumType) \
    inline EnumType operator|(EnumType Lhs, EnumType Rhs) \
    { \
        return static_cast<EnumType>(static_cast<uint32_t>(Lhs) | static_cast<uint32_t>(Rhs)); \
    } \
    \
    inline bool operator&(EnumType Lhs, EnumType Rhs) \
    { \
        return (static_cast<uint32_t>(Lhs) & static_cast<uint32_t>(Rhs)) != 0; \
    }


namespace LE::RHI
{
constexpr uint32 DEFAULT_MIN_ALIGNMENT = 16;
constexpr uint32 DEFAULT_UNIFORM_ALIGNMENT = 256;

inline uint32 GlobalStorageAlignment = DEFAULT_MIN_ALIGNMENT;
inline uint32 GlobalUniformAlignment = DEFAULT_UNIFORM_ALIGNMENT;

enum class RHIDeviceType : uint8
{
    None,

    Vulkan,

    Count,
};

enum class RHIResourceType : uint8
{
    None,

    Buffer,

    BufferSubAllocation,
    GlobalBufferChannel,

    Image,
    ImageView,

    VirtualMemoryBlock,
    VirtualMemoryAllocation,

    CommandList,

    PipelineLayout,
    PipelineObject,

    Window,

    Count,
};

/**
 * @brief Buffer usage type. It abstracts boilerplate API specific configuration for buffer
 */
enum class RHIBufferUsageType : uint8
{
    Unknown,

    /// Global buffer for mesh data. GPU-only. Use stage buffer to transfer data into it
    MeshGlobal,

    /// Global buffer for material data. GPU-only. Use stage buffer to transfer data into it
    MaterialGlobal,

    /// Global buffer used for frame data. CPU-to-GPU which is updated per frame. Memory is mapped
    DynamicFrameData,

    /// Staging buffer
    UploadStaging,

    Count,
};

enum class RHIGlobalBufferChannelType : uint8
{
    Unknown = 0,
    Position,
    Normal,
    TexCoord,
    Index,
    Count
};

enum class PrimitiveType : uint8
{
    Unknown,

    TriangleList,
};

enum class RHICommandListType : uint8
{
    Unknown,

    Graphics,
    Transfer,
};

enum class RHIShaderStage : uint32
{
    None = 0,
    Vertex = 1 << 0,
    Fragment = 1 << 1,
    Compute = 1 << 2,

    // Combined utility helpers
    AllGraphics = Vertex | Fragment
};
EnableBitwiseOperations(RHIShaderStage)

enum class RHIFormat : uint8
{
    None = 0,

    // 8-bit per channel formats
    R8_Unorm,
    R8G8_Unorm,
    R8G8B8A8_Unorm,
    R8G8B8A8_Srgb,
    B8G8R8A8_Unorm,
    B8G8R8A8_Srgb,

    // 16-bit float formats (HDR/render targets)
    R16_Float,
    R16G16_Float,
    R16G16B16A16_Float,

    // 32-bit float formats (Data/positions)
    R32_Float,
    R32G32_Float,
    R32G32B32_Float,
    R32G32B32A32_Float,

    // Integer / Index / ID formats
    R16_Uint,
    R32_Uint,

    // Packed / HDR formats
    A2B10G10R10_Unorm,
    B10G11R11_Float,

    // Common Depth/Stencil formats
    D16_Unorm,
    D24_Unorm_S8_Uint,
    D32_Float,
    D32_Float_S8_Uint,
};

enum class RHIImageViewType : uint8
{
    None = 0,
    View1D,
    View2D,
    View3D,
    ViewCube,
};

enum class RHIImageAspectFlags : uint32
{
    Color = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 2,
};
EnableBitwiseOperations(RHIImageAspectFlags)

enum class RHIImageUsageFlag : uint32
{
    Sampled = 1 << 0,
    Storage = 1 << 1,
    ColorAttachment = 1 << 2,
    DepthStencil = 1 << 3,
    TransferSrc = 1 << 4,
    TransferDst = 1 << 5,
};
EnableBitwiseOperations(RHIImageUsageFlag)

enum class RHILoadOp : uint8
{
    Load,
    Clear,
    Ignore
};

enum class RHIStoreOp : uint8
{
    Store,
    Ignore
};

enum class RHIPipelineStageFlags : uint64
{
    None = 0,
    Top = 1ULL << 0,
    ColorTarget = 1ULL << 1,
    DepthTarget = 1ULL << 2,
    ComputeShader = 1ULL << 3,
    FragmentShader = 1ULL << 4,
    Transfer = 1ULL << 5,
    Bottom = 1ULL << 6,
};
EnableBitwiseOperations(RHIPipelineStageFlags)

enum class RHIAccessFlags : uint64
{
    None = 0,
    Read = 1ULL << 0,
    Write = 1ULL << 1,
    ColorWrite = 1ULL << 2,
    ColorRead = 1ULL << 3,
    DepthWrite = 1ULL << 4,
    DepthRead = 1ULL << 5,
    ShaderRead = 1ULL << 6,
    TransferWrite = 1ULL << 7,
};
EnableBitwiseOperations(RHIAccessFlags)

enum class RHIImageLayout : uint32
{
    None,
    General,
    Attachment,
    ColorAttachment,
    DepthAttachment,
    ShaderReadOnly,
    Present,
};

}