#pragma once

#include <volk/volk.h>

#include "Core.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"

namespace LE::RHI::Vulkan
{
static inline bool CheckVkResult(VkResult result)
{
    const bool resultCheck = result == VK_SUCCESS;
    LE_ASSERT_DESC(resultCheck, "Vulkan call returned an error")
    return resultCheck;
}

static inline bool CheckVkResult(bool result)
{
    LE_ASSERT_DESC(result, "Vulkan call returned an error")
    return result;
}

VkShaderStageFlags ShaderStage(RHIShaderStage rhiStages);
VkShaderStageFlagBits ShaderStageFlagBit(RHIShaderStage rhiStages);
RHIFormat MapVkFormat(VkFormat format);
VkFormat MapRHIFormat(RHIFormat format);
VkImageViewType MapViewType(RHIImageViewType type);
VkImageAspectFlags MapAspectFlags(RHIImageAspectFlags flags);
VkImageUsageFlags MapUsageFlags(RHIImageUsageFlag flags);
VkPipelineStageFlags2 MapPipelineStages(RHIPipelineStageFlags flags);
VkAccessFlags2 MapAccessFlags(RHIAccessFlags flags);
VkImageLayout MapImageLayout(RHIImageLayout layout);
VkImageSubresourceRange MapSubresourceRange(const RHISubresourceRange& range);
VkAttachmentLoadOp MapAttachmentLoadOp(RHILoadOp op);
VkAttachmentStoreOp MapAttachmentStoreOp(RHIStoreOp op);
VkRenderingAttachmentInfo MapRenderingAttachmentInfo(const RHIRenderingAttachmentDesc& desc);
VkDescriptorType MapDescriptorType(RHIDescriptorType type);
VkDescriptorBindingFlags MapDescriptorBindingFlags(RHIDescriptorBindingFlags flags);
VkDescriptorSetLayoutCreateFlags MapDescriptorSetLayoutCreateFlags(RHIDescriptorSetLayoutCreateFlags flags);
VkDescriptorPoolCreateFlags MapDescriptorPoolCreateFlags(RHIPoolCreateFlags flags);
VkImageSubresourceLayers MapSubresourceLayers(const RHIImageSubresourceLayers& layers);

VkImageType GetImageTypeFromImageDesc(const RHIImageDesc& Desc);

}
