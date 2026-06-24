#pragma once

#include <vector>

#include "VulkanFwd.h"

namespace LE::RHI::Vulkan
{
struct VulkanThreadResources
{
	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> AvailableCommandBuffers;
	std::vector<VkCommandBuffer> ActiveCommandBuffers;
};
}