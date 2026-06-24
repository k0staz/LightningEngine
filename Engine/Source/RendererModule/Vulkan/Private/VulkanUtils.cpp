#include "VulkanUtils.h"

#include "VulkanResources.h"

namespace LE::RHI::Vulkan
{
VkShaderStageFlags ShaderStage(RHIShaderStage rhiStages)
{
    VkShaderStageFlags vkFlags = 0;
    if (rhiStages & RHIShaderStage::Vertex) vkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (rhiStages & RHIShaderStage::Fragment) vkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (rhiStages & RHIShaderStage::Compute) vkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
    return vkFlags;
}

VkShaderStageFlagBits ShaderStageFlagBit(RHIShaderStage rhiStages)
{
    VkShaderStageFlagBits vkFlags = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    switch (rhiStages)
    {
    case RHIShaderStage::Vertex: vkFlags = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case RHIShaderStage::Fragment: vkFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    case RHIShaderStage::Compute: vkFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        break;
    case RHIShaderStage::None:
        break;
    case RHIShaderStage::AllGraphics:
        break;
    }

    return vkFlags;
}

RHIFormat MapVkFormat(VkFormat format)
{
    switch (format)
    {
    // 8-bit per channel formats
    case VK_FORMAT_R8_UNORM: return RHIFormat::R8_Unorm;
    case VK_FORMAT_R8G8_UNORM: return RHIFormat::R8G8_Unorm;
    case VK_FORMAT_R8G8B8A8_UNORM: return RHIFormat::R8G8B8A8_Unorm;
    case VK_FORMAT_R8G8B8A8_SRGB: return RHIFormat::R8G8B8A8_Srgb;
    case VK_FORMAT_B8G8R8A8_UNORM: return RHIFormat::B8G8R8A8_Unorm;
    case VK_FORMAT_B8G8R8A8_SRGB: return RHIFormat::B8G8R8A8_Srgb;

    // 16-bit float formats
    case VK_FORMAT_R16_SFLOAT: return RHIFormat::R16_Float;
    case VK_FORMAT_R16G16_SFLOAT: return RHIFormat::R16G16_Float;
    case VK_FORMAT_R16G16B16A16_SFLOAT: return RHIFormat::R16G16B16A16_Float;

    // 32-bit float formats
    case VK_FORMAT_R32_SFLOAT: return RHIFormat::R32_Float;
    case VK_FORMAT_R32G32_SFLOAT: return RHIFormat::R32G32_Float;
    case VK_FORMAT_R32G32B32_SFLOAT: return RHIFormat::R32G32B32_Float;
    case VK_FORMAT_R32G32B32A32_SFLOAT: return RHIFormat::R32G32B32A32_Float;

    // Integer / Index formats
    case VK_FORMAT_R16_UINT: return RHIFormat::R16_Uint;
    case VK_FORMAT_R32_UINT: return RHIFormat::R32_Uint;

    // Packed / HDR formats
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return RHIFormat::A2B10G10R10_Unorm;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return RHIFormat::B10G11R11_Float;

    // Depth/Stencil formats
    case VK_FORMAT_D16_UNORM: return RHIFormat::D16_Unorm;
    case VK_FORMAT_D24_UNORM_S8_UINT: return RHIFormat::D24_Unorm_S8_Uint;
    case VK_FORMAT_D32_SFLOAT: return RHIFormat::D32_Float;
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return RHIFormat::D32_Float_S8_Uint;

    default: return RHIFormat::None;
    }
}

VkFormat MapRHIFormat(RHIFormat format)
{
    switch (format)
    {
    // 8-bit per channel formats
    case RHIFormat::R8_Unorm: return VK_FORMAT_R8_UNORM;
    case RHIFormat::R8G8_Unorm: return VK_FORMAT_R8G8_UNORM;
    case RHIFormat::R8G8B8A8_Unorm: return VK_FORMAT_R8G8B8A8_UNORM;
    case RHIFormat::R8G8B8A8_Srgb: return VK_FORMAT_R8G8B8A8_SRGB;
    case RHIFormat::B8G8R8A8_Unorm: return VK_FORMAT_B8G8R8A8_UNORM;
    case RHIFormat::B8G8R8A8_Srgb: return VK_FORMAT_B8G8R8A8_SRGB;

    // 16-bit float formats
    case RHIFormat::R16_Float: return VK_FORMAT_R16_SFLOAT;
    case RHIFormat::R16G16_Float: return VK_FORMAT_R16G16_SFLOAT;
    case RHIFormat::R16G16B16A16_Float: return VK_FORMAT_R16G16B16A16_SFLOAT;

    // 32-bit float formats
    case RHIFormat::R32_Float: return VK_FORMAT_R32_SFLOAT;
    case RHIFormat::R32G32_Float: return VK_FORMAT_R32G32_SFLOAT;
    case RHIFormat::R32G32B32_Float: return VK_FORMAT_R32G32B32_SFLOAT;
    case RHIFormat::R32G32B32A32_Float: return VK_FORMAT_R32G32B32A32_SFLOAT;

    // Integer / Index formats
    case RHIFormat::R16_Uint: return VK_FORMAT_R16_UINT;
    case RHIFormat::R32_Uint: return VK_FORMAT_R32_UINT;

    // Packed / HDR formats
    case RHIFormat::A2B10G10R10_Unorm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case RHIFormat::B10G11R11_Float: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

    // Depth/Stencil formats
    case RHIFormat::D16_Unorm: return VK_FORMAT_D16_UNORM;
    case RHIFormat::D24_Unorm_S8_Uint: return VK_FORMAT_D24_UNORM_S8_UINT;
    case RHIFormat::D32_Float: return VK_FORMAT_D32_SFLOAT;
    case RHIFormat::D32_Float_S8_Uint: return VK_FORMAT_D32_SFLOAT_S8_UINT;

    default: return VK_FORMAT_UNDEFINED;
    }
}

VkImageViewType MapViewType(RHIImageViewType type)
{
    switch (type)
    {
    case RHIImageViewType::View1D: return VK_IMAGE_VIEW_TYPE_1D;
    case RHIImageViewType::View2D: return VK_IMAGE_VIEW_TYPE_2D;
    case RHIImageViewType::View3D: return VK_IMAGE_VIEW_TYPE_3D;
    case RHIImageViewType::ViewCube: return VK_IMAGE_VIEW_TYPE_CUBE;
    case RHIImageViewType::None:
        break;
    }
    return VK_IMAGE_VIEW_TYPE_2D;
}

VkImageAspectFlags MapAspectFlags(RHIImageAspectFlags flags)
{
    VkImageAspectFlags vkFlags = 0;
    if (flags & RHIImageAspectFlags::Color) vkFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
    if (flags & RHIImageAspectFlags::Depth) vkFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if (flags & RHIImageAspectFlags::Stencil) vkFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    return vkFlags;
}

VkImageUsageFlags MapUsageFlags(RHIImageUsageFlag flags)
{
    VkImageUsageFlags vkFlags = 0;
    if (flags & RHIImageUsageFlag::TransferSrc) vkFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (flags & RHIImageUsageFlag::TransferDst) vkFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (flags & RHIImageUsageFlag::Sampled) vkFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (flags & RHIImageUsageFlag::Storage) vkFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (flags & RHIImageUsageFlag::ColorAttachment) vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (flags & RHIImageUsageFlag::DepthStencil) vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    return vkFlags;
}

VkPipelineStageFlags2 MapPipelineStages(RHIPipelineStageFlags flags)
{
    if (flags == RHIPipelineStageFlags::None)
    {
        return VK_PIPELINE_STAGE_2_NONE;
    }

    VkPipelineStageFlags2 vkFlags = 0;
    if (flags & RHIPipelineStageFlags::Top) vkFlags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    if (flags & RHIPipelineStageFlags::ColorTarget) vkFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (flags & RHIPipelineStageFlags::DepthTarget) vkFlags |= (VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT);
    if (flags & RHIPipelineStageFlags::ComputeShader) vkFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    if (flags & RHIPipelineStageFlags::FragmentShader) vkFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    if (flags & RHIPipelineStageFlags::Transfer) vkFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    if (flags & RHIPipelineStageFlags::Bottom) vkFlags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

    return vkFlags;
}

VkAccessFlags2 MapAccessFlags(RHIAccessFlags flags)
{
    if (flags == RHIAccessFlags::None)
    {
        return VK_ACCESS_2_NONE;
    }

    VkAccessFlags2 vkFlags = 0;
    if (flags & RHIAccessFlags::Read) vkFlags |= VK_ACCESS_2_MEMORY_READ_BIT;
    if (flags & RHIAccessFlags::Write) vkFlags |= VK_ACCESS_2_MEMORY_WRITE_BIT;
    if (flags & RHIAccessFlags::ColorWrite) vkFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    if (flags & RHIAccessFlags::ColorRead) vkFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    if (flags & RHIAccessFlags::DepthWrite) vkFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (flags & RHIAccessFlags::DepthRead) vkFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if (flags & RHIAccessFlags::ShaderRead) vkFlags |= VK_ACCESS_2_SHADER_READ_BIT;
    if (flags & RHIAccessFlags::TransferWrite) vkFlags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;

    return vkFlags;
}

VkImageLayout MapImageLayout(RHIImageLayout layout)
{
    switch (layout)
    {
    case RHIImageLayout::None: return VK_IMAGE_LAYOUT_UNDEFINED;
    case RHIImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
    case RHIImageLayout::Attachment: return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    case RHIImageLayout::ColorAttachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case RHIImageLayout::DepthAttachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case RHIImageLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case RHIImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageSubresourceRange MapSubresourceRange(const RHISubresourceRange& range)
{
    VkImageSubresourceRange vkRange = {};
    vkRange.aspectMask = MapAspectFlags(range.Aspect);
    vkRange.baseMipLevel = range.BaseMipLevel;
    vkRange.levelCount = range.NumMipLevels;
    vkRange.baseArrayLayer = range.BaseArraySlice;
    vkRange.layerCount = range.NumArraySlices;
    return vkRange;
}

VkAttachmentLoadOp MapAttachmentLoadOp(RHILoadOp op)
{
    switch (op)
    {
    case RHILoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
    case RHILoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case RHILoadOp::Ignore: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

VkAttachmentStoreOp MapAttachmentStoreOp(RHIStoreOp op)
{
    switch (op)
    {
    case RHIStoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
    case RHIStoreOp::Ignore: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

VkRenderingAttachmentInfo MapRenderingAttachmentInfo(const RHIRenderingAttachmentDesc& desc)
{
    VkRenderingAttachmentInfo vkAttachmentInfo = {.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};

    vkAttachmentInfo.imageView = ResourceCast(desc.Attachment.GetPointer())->GetVkImageView();
    vkAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    vkAttachmentInfo.loadOp = MapAttachmentLoadOp(desc.LoadOp);
    vkAttachmentInfo.storeOp = MapAttachmentStoreOp(desc.StoreOp);
    std::memcpy(&vkAttachmentInfo.clearValue, &desc.ClearValue, sizeof(vkAttachmentInfo.clearValue));

    return vkAttachmentInfo;
}

VkImageType GetImageTypeFromImageDesc(const RHIImageDesc& Desc)
{
    if (Desc.Depth > 1)
    {
        return VK_IMAGE_TYPE_3D;
    }

    if (Desc.Height == 1 && Desc.Width == 1)
    {
        return VK_IMAGE_TYPE_1D;
    }

    return VK_IMAGE_TYPE_2D;
}
}
