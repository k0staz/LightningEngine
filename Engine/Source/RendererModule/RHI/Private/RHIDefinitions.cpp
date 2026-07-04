#include "RHIDefinitions.h"

namespace LE::RHI
{
RHIFormat MapFromVkFormat(uint32 VkFormat)
{
    switch (VkFormat)
    {
    // 8-bit formats
    case 9: return RHIFormat::R8_Unorm; // VK_FORMAT_R8_UNORM
    case 23: return RHIFormat::R8G8_Unorm; // VK_FORMAT_R8G8_UNORM
    case 37: return RHIFormat::R8G8B8A8_Unorm; // VK_FORMAT_R8G8B8A8_UNORM
    case 43: return RHIFormat::R8G8B8A8_Srgb; // VK_FORMAT_R8G8B8A8_SRGB
    case 44: return RHIFormat::B8G8R8A8_Unorm; // VK_FORMAT_B8G8R8A8_UNORM
    case 50: return RHIFormat::B8G8R8A8_Srgb; // VK_FORMAT_B8G8R8A8_SRGB

    // 16-bit float formats
    case 76: return RHIFormat::R16_Float; // VK_FORMAT_R16_SFLOAT
    case 83: return RHIFormat::R16G16_Float; // VK_FORMAT_R16G16_SFLOAT
    case 97: return RHIFormat::R16G16B16A16_Float; // VK_FORMAT_R16G16B16A16_SFLOAT

    // 32-bit float formats
    case 100: return RHIFormat::R32_Float; // VK_FORMAT_R32_SFLOAT
    case 107: return RHIFormat::R32G32_Float; // VK_FORMAT_R32G32_SFLOAT
    case 114: return RHIFormat::R32G32B32_Float; // VK_FORMAT_R32G32B32_SFLOAT
    case 121: return RHIFormat::R32G32B32A32_Float; // VK_FORMAT_R32G32B32A32_SFLOAT

    // Integer formats
    case 74: return RHIFormat::R16_Uint; // VK_FORMAT_R16_UINT
    case 98: return RHIFormat::R32_Uint; // VK_FORMAT_R32_UINT

    // Packed formats
    case 58: return RHIFormat::A2B10G10R10_Unorm; // VK_FORMAT_A2B10G10R10_UNORM_PACK32
    case 122: return RHIFormat::B10G11R11_Float; // VK_FORMAT_B10G11R11_UFLOAT_PACK32

    // Depth/Stencil formats
    case 124: return RHIFormat::D16_Unorm; // VK_FORMAT_D16_UNORM

    default: return RHIFormat::None;
    }
    return RHIFormat::None;
}
}
