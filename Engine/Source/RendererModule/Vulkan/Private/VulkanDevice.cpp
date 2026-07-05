#include "VulkanDevice.h"

#define VOLK_IMPLEMENTATION
#include <volk/volk.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <set>

#include "Core.h"
#include "CoreDefinitions.h"
#include "RenderCore.h"
#include "VulkanResources.h"
#include "Multithreading/Thread.h"
#include "VulkanUtils.h"
#include "tracy/Tracy.hpp"

namespace LE::RHI::Vulkan
{
VulkanDevice::VulkanDevice()
    : RHIDevice(RHIDeviceType::Vulkan)
{
    for (uint8 i = 0; i < DEFAULT_FRAMES_IN_FLIGHT; ++i)
    {
        TransferFrameSemaphoreValues[i] = 0;
        GraphicsFrameFences[i] = nullptr;
    }
}

void VulkanDevice::Initialize()
{
    CheckVkResult(SDL_Init(SDL_INIT_VIDEO));
    CheckVkResult(SDL_Vulkan_LoadLibrary(NULL));
    CheckVkResult(volkInitialize());

    // Instance set-up
    VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pApplicationName = "LEngine", .apiVersion = VK_API_VERSION_1_3};
    uint32 instanceExtensionsCount = 0;
    char const* const* instanceExtensions{SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount)};
    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = instanceExtensionsCount,
        .ppEnabledExtensionNames = instanceExtensions,
    };
    CheckVkResult(vkCreateInstance(&instanceCI, nullptr, &Instance));
    volkLoadInstance(Instance);

    // Device
    uint32_t deviceCount{0};
    CheckVkResult(vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    CheckVkResult(vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data()));

    PhysicalDevice = VK_NULL_HANDLE;
    // Select first discrete GPU
    for (auto& device : devices)
    {
        VkPhysicalDeviceProperties2 deviceProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(device, &deviceProperties);

        if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            PhysicalDevice = device;
            break;
        }
    }

    // Fallback to first one, if discreate GPU was not found
    if (PhysicalDevice == VK_NULL_HANDLE)
    {
        LE_INFO("Discreate GPU was not found, fallback to first available device");
        PhysicalDevice = devices[0];
    }

    std::vector<VkFormat> depthFormatList{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    VkFormat depthFormat{VK_FORMAT_UNDEFINED};
    for (VkFormat& format : depthFormatList)
    {
        VkFormatProperties2 formatProperties{.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        vkGetPhysicalDeviceFormatProperties2(PhysicalDevice, format, &formatProperties);
        if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depthFormat = format;
            break;
        }
    }

    DepthFormat = MapVkFormat(depthFormat);
    SwapChainFormat = MapVkFormat(VK_FORMAT_B8G8R8A8_SRGB);

    VkPhysicalDeviceProperties2 deviceProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    vkGetPhysicalDeviceProperties2(PhysicalDevice, &deviceProperties);
    LE_INFO("Selected device: {}", deviceProperties.properties.deviceName);

    // Required alignment for this device
    const uint32 deviceStorageAlign = static_cast<uint32>(deviceProperties.properties.limits.minStorageBufferOffsetAlignment);
    const uint32 deviceUniformAlign = static_cast<uint32_t>(deviceProperties.properties.limits.minUniformBufferOffsetAlignment);

    RHI::GlobalStorageAlignment = std::max(RHI::DEFAULT_MIN_ALIGNMENT, deviceStorageAlign);
    RHI::GlobalUniformAlignment = std::max(RHI::DEFAULT_UNIFORM_ALIGNMENT, deviceUniformAlign);

    // Set-up queue
    uint32 queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32 graphicsQueueFamily = UINT32_MAX;
    uint32 transferQueueFamily = UINT32_MAX;
    for (size_t i = 0; i < queueFamilies.size(); i++)
    {
        const auto& flags = queueFamilies[i].queueFlags;

        if (flags & VK_QUEUE_GRAPHICS_BIT)
        {
            if (graphicsQueueFamily == UINT32_MAX)
            {
                graphicsQueueFamily = i;
            }
        }

        if (flags & VK_QUEUE_TRANSFER_BIT)
        {
            const bool isDedicated = !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT);
            if (isDedicated)
            {
                transferQueueFamily = i;
            }
            else if (transferQueueFamily == UINT32_MAX)
            {
                transferQueueFamily = i;
            }
        }
    }
    CheckVkResult(SDL_Vulkan_GetPresentationSupport(Instance, PhysicalDevice, graphicsQueueFamily));

    if (transferQueueFamily == UINT32_MAX)
    {
        transferQueueFamily = graphicsQueueFamily;
    }

    TransferQueueFamilyIndex = transferQueueFamily;
    GraphicsQueueFamilyIndex = graphicsQueueFamily;

    const std::set<uint32> foundQueueFamilies{graphicsQueueFamily, transferQueueFamily};

    // Logical device
    const float qfpriorities{1.0f};

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (const auto& queueFamily : foundQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &qfpriorities
        };

        queueCreateInfos.push_back(queueCI);
    }

    VkPhysicalDeviceVulkan11Features enabledVk11Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = nullptr,
        .storageBuffer16BitAccess = VK_TRUE
    };

    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &enabledVk11Features,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingSampledImageUpdateAfterBind = true,
        .descriptorBindingPartiallyBound = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .timelineSemaphore = true,
        .bufferDeviceAddress = true
    };
    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledVk12Features,
        .synchronization2 = true,
        .dynamicRendering = true
    };

    VkPhysicalDeviceFeatures enabledVk10Features{.samplerAnisotropy = VK_TRUE, .shaderInt64 = true};

    const std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME};

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk13Features,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVk10Features
    };

    // Command queues
    CheckVkResult(vkCreateDevice(PhysicalDevice, &deviceCI, nullptr, &Device));
    vkGetDeviceQueue(Device, graphicsQueueFamily, 0, &GraphicsQueue);
    vkGetDeviceQueue(Device, transferQueueFamily, 0, &TransferQueue);

    for (uint32 workerTaskIdx = 0; workerTaskIdx < DEFAULT_TASK_WORKER_THREADS + 1; ++workerTaskIdx)
    {
        for (uint32 frameIdx = 0; frameIdx < DEFAULT_FRAMES_IN_FLIGHT; ++frameIdx)
        {
            VulkanThreadResources& graphicsThreadResource = GraphicsThreadResources[workerTaskIdx][frameIdx];
            VkCommandPoolCreateInfo graphicCommandPoolCI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = graphicsQueueFamily
            };
            CheckVkResult(vkCreateCommandPool(Device, &graphicCommandPoolCI, nullptr, &graphicsThreadResource.CommandPool));

            VulkanThreadResources& transferThreadResource = TransferThreadResources[workerTaskIdx][frameIdx];
            VkCommandPoolCreateInfo transferCommandPoolCI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = transferQueueFamily
            };
            CheckVkResult(vkCreateCommandPool(Device, &transferCommandPoolCI, nullptr, &transferThreadResource.CommandPool));
        }
    }

    // Synchronization objects
    VkSemaphoreTypeCreateInfo semaphoreTypeCI{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
    };

    VkSemaphoreCreateInfo semaphoreCI{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCI,
    };
    CheckVkResult(vkCreateSemaphore(Device, &semaphoreCI, nullptr, &TransferTimelineSemaphore));

    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    VkSemaphoreCreateInfo genericSemaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (uint32 frameIdx = 0; frameIdx < DEFAULT_FRAMES_IN_FLIGHT; ++frameIdx)
    {
        CheckVkResult(vkCreateFence(Device, &fenceCI, nullptr, &GraphicsFrameFences[frameIdx]));
        CheckVkResult(vkCreateSemaphore(Device, &genericSemaphoreCI, nullptr, &SwapchainImageAvailableSemaphores[frameIdx]));
        CheckVkResult(vkCreateSemaphore(Device, &genericSemaphoreCI, nullptr, &RenderCompleteSemaphores[frameIdx]));
    }

    // VMA
    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr, .vkGetDeviceProcAddr = vkGetDeviceProcAddr, .vkCreateImage = vkCreateImage
    };
    VmaAllocatorCreateInfo allocatorCI{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = PhysicalDevice,
        .device = Device,
        .pVulkanFunctions = &vkFunctions,
        .instance = Instance
    };
    CheckVkResult(vmaCreateAllocator(&allocatorCI, &Allocator));
}

void VulkanDevice::Shutdown()
{
    for (uint32 workerTaskIdx = 0; workerTaskIdx < DEFAULT_TASK_WORKER_THREADS + 1; ++workerTaskIdx)
    {
        for (uint32 frameIdx = 0; frameIdx < DEFAULT_FRAMES_IN_FLIGHT; ++frameIdx)
        {
            VulkanThreadResources& graphicsThreadResource = GraphicsThreadResources[workerTaskIdx][frameIdx];
            vkDestroyCommandPool(Device, graphicsThreadResource.CommandPool, nullptr);

            VulkanThreadResources& transferThreadResource = TransferThreadResources[workerTaskIdx][frameIdx];
            vkDestroyCommandPool(Device, transferThreadResource.CommandPool, nullptr);
        }
    }

    for (uint32 frameIdx = 0; frameIdx < DEFAULT_FRAMES_IN_FLIGHT; ++frameIdx)
    {
        vkDestroySemaphore(Device, RenderCompleteSemaphores[frameIdx], nullptr);
        vkDestroySemaphore(Device, SwapchainImageAvailableSemaphores[frameIdx], nullptr);
        vkDestroyFence(Device, GraphicsFrameFences[frameIdx], nullptr);
    }

    vkDestroySemaphore(Device, TransferTimelineSemaphore, nullptr);

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();

    vmaDestroyAllocator(Allocator);
    vkDestroyDevice(Device, nullptr);
    vkDestroyInstance(Instance, nullptr);
}

void VulkanDevice::WaitIdle()
{
    vkDeviceWaitIdle(Device);
}

void VulkanDevice::BeginFrame()
{
    ZoneScopedN("VulkanDevice::BeginFrame");
    if (!Thread::IsRenderThread())
    {
        LE_ASSERT_DESC(false, "Only render thread should begin render frame")
        return;
    }

    const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;

    VkFence graphicsFrameFence = GraphicsFrameFences[frameIdx];
    CheckVkResult(vkWaitForFences(Device, 1, &graphicsFrameFence, VK_TRUE, UINT64_MAX));
    vkResetFences(Device, 1, &graphicsFrameFence);

    uint64 transferWaitValue = TransferFrameSemaphoreValues[frameIdx];
    LE_INFO("transferWaitValue: {}. FrameIdx: {}", transferWaitValue, frameIdx);
    if (transferWaitValue > 0)
    {
        VkSemaphoreWaitInfo waitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1,
            .pSemaphores = &TransferTimelineSemaphore,
            .pValues = &transferWaitValue
        };

        CheckVkResult(vkWaitSemaphores(Device, &waitInfo, UINT64_MAX));
    }

    auto resetPoolsLambda = [this](VulkanThreadResources& resources)
    {
        vkResetCommandPool(Device, resources.CommandPool, 0);
        resources.AvailableCommandBuffers.insert(
            resources.AvailableCommandBuffers.end(),
            resources.ActiveCommandBuffers.begin(),
            resources.ActiveCommandBuffers.end());
        resources.ActiveCommandBuffers.clear();
    };
    for (uint32 threadIdx = 0; threadIdx < DEFAULT_TASK_WORKER_THREADS + 1; ++threadIdx)
    {
        resetPoolsLambda(GraphicsThreadResources[threadIdx][frameIdx]);
        resetPoolsLambda(TransferThreadResources[threadIdx][frameIdx]);
    }
}

RefCountingPtr<RHIBuffer> VulkanDevice::CreateBuffer(RHIBufferDescription BufferDesc)
{
    RHIBuffer* result = nullptr;
    if (!BufferDesc.IsValid())
    {
        LE_ASSERT_DESC(false, "Invalid buffer description");
        return result;
    }

    VkBufferCreateInfo bufferCI = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCI.size = BufferDesc.Size;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    switch (BufferDesc.UsageType)
    {
    case RHIBufferUsageType::MeshGlobal:
    case RHIBufferUsageType::MaterialGlobal:
        bufferCI.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        break;
    case RHIBufferUsageType::DynamicFrameData:
        bufferCI.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;
    case RHIBufferUsageType::UploadStaging:
        bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;
    case RHIBufferUsageType::Unknown:
    case RHIBufferUsageType::Count:
    default:
        LE_ASSERT_DESC(false, "Invalid buffer usage")
        return result;
        break;
    }

    VmaAllocationInfo resultInfo;
    VkBuffer buffer;
    VmaAllocation allocation;
    CheckVkResult(vmaCreateBuffer(Allocator, &bufferCI, &allocInfo, &buffer, &allocation, &resultInfo));

    VkDeviceAddress deviceAddress = 0;
    if (bufferCI.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        VkBufferDeviceAddressInfo addressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
        addressInfo.buffer = buffer;
        deviceAddress = vkGetBufferDeviceAddress(Device, &addressInfo);
    }

    switch (BufferDesc.UsageType)
    {
    case RHIBufferUsageType::MeshGlobal:
    case RHIBufferUsageType::MaterialGlobal:
        {
            VmaVirtualBlockCreateInfo blockCI = {};
            blockCI.size = BufferDesc.Size;

            VmaVirtualBlock block;
            CheckVkResult(vmaCreateVirtualBlock(&blockCI, &block));
            result = new VulkanGlobalBuffer(buffer, allocation, deviceAddress, block, BufferDesc);
            break;
        }
    case RHIBufferUsageType::DynamicFrameData:
    case RHIBufferUsageType::UploadStaging:
        {
            result = new VulkanLinearBuffer(buffer, allocation, deviceAddress, resultInfo.pMappedData, BufferDesc);
            break;
        }
    case RHIBufferUsageType::Unknown:
    case RHIBufferUsageType::Count:
    default:
        break;
    }
    return result;
}

void VulkanDevice::DestroyBuffer(RefCountingPtr<RHIBuffer> Buffer)
{
    if (!Buffer.IsValid() || !Buffer->IsValid())
    {
        LE_ASSERT_DESC(false, "Invalid buffer")
        return;
    }

    LE_ASSERT_DESC(Buffer.GetRefCount() > 1, "Buffer is still referenced")

    VulkanBuffer* vulkanBuffer = nullptr;

    if (Buffer->IsGlobalBuffer())
    {
        vulkanBuffer = ResourceCast(Buffer->GetAs<RHIGlobalBuffer>());
    }
    else
    {
        vulkanBuffer = ResourceCast(Buffer->GetAs<RHILinearBuffer>());
    }

    vmaDestroyBuffer(Allocator, vulkanBuffer->GetHandle(), vulkanBuffer->GetAllocation());
    vulkanBuffer->Reset();
}

RefCountingPtr<RHICommandList> VulkanDevice::CreateCommandList(RHICommandListType ListType)
{
    uint32 threadIdx = 0;
    if (Thread::IsRenderThread())
    {
        threadIdx = DEFAULT_TASK_WORKER_THREADS;
    }
    else if (Thread::IsTaskThread())
    {
        threadIdx = Thread::GetWorkerTaskThreadIndex();
    }
    else
    {
        LE_ASSERT_DESC(false, "Accessed by invalid thread. Only Task Threads and Render Threads are allowed")
        return nullptr;
    }

    uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;

    VulkanThreadResources* threadResources = nullptr;
    switch (ListType)
    {
    case RHICommandListType::Graphics:
        threadResources = &GraphicsThreadResources[threadIdx][frameIdx];
        break;
    case RHICommandListType::Transfer:
        threadResources = &TransferThreadResources[threadIdx][frameIdx];
        break;
    case RHICommandListType::Unknown:
    default:
        LE_ASSERT_DESC(false, "Invalid command list type")
        break;
    }

    if (!threadResources)
    {
        LE_ASSERT_DESC(false, "Invalid thread resources")
        return nullptr;
    }

    std::vector<VkCommandBuffer>& availableCommandBuffers = threadResources->AvailableCommandBuffers;
    std::vector<VkCommandBuffer>& activeCommandBuffers = threadResources->ActiveCommandBuffers;

    VkCommandBuffer commandBuffer = nullptr;
    if (!availableCommandBuffers.empty())
    {
        commandBuffer = availableCommandBuffers.back();
        availableCommandBuffers.pop_back();
    }
    else
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = threadResources->CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        CheckVkResult(vkAllocateCommandBuffers(Device, &allocInfo, &commandBuffer));
    }

    if (!commandBuffer)
    {
        LE_ASSERT_DESC(false, "Failed to get command buffer")
        return nullptr;
    }

    activeCommandBuffers.push_back(commandBuffer);
    return new VulkanCommandList{commandBuffer, ListType};
}

void VulkanDevice::SubmitCommandList(RHICommandListType ListType, const std::vector<RefCountingPtr<RHICommandList>>& CommandLists,
                                     uint32 SwapchainImageIdx)
{
    if (!Thread::IsRenderThread())
    {
        LE_ASSERT_DESC(false, "Submitting command lists is only allowed from render thread")
        return;
    }

    if (CommandLists.empty())
    {
        return;
    }


    std::vector<VkCommandBufferSubmitInfo> cmdBufferInfos;
    cmdBufferInfos.reserve(CommandLists.size());
    for (auto& recordedBuffer : CommandLists)
    {
        VulkanCommandList* vulkanCommandList = ResourceCast(recordedBuffer.GetPointer());

        VkCommandBufferSubmitInfo cmdInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = vulkanCommandList->GetVkCommandBuffer(),
            .deviceMask = 0
        };

        cmdBufferInfos.push_back(cmdInfo);
    }

    if (ListType == RHICommandListType::Transfer)
    {
        uint64 targetSignalValue = NextTransferValue.load(std::memory_order_relaxed);

        VkSemaphoreSubmitInfo signalInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = TransferTimelineSemaphore,
            .value = targetSignalValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
            .deviceIndex = 0
        };

        VkSubmitInfo2 submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 0,
            .pWaitSemaphoreInfos = nullptr,
            .commandBufferInfoCount = static_cast<uint32>(cmdBufferInfos.size()),
            .pCommandBufferInfos = cmdBufferInfos.data(),
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signalInfo
        };

        const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
        vkQueueSubmit2(TransferQueue, 1, &submitInfo, nullptr);

        LE_INFO("Submit transfer command list. FrameIdx: {}", frameIdx);

        TransferFrameSemaphoreValues[frameIdx] = targetSignalValue;
        NextTransferValue.fetch_add(1, std::memory_order_relaxed);
    }
    else if (ListType == RHICommandListType::Graphics)
    {
        const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;

        VkSemaphoreSubmitInfo waitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = SwapchainImageAvailableSemaphores[frameIdx],
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .deviceIndex = 0
        };

        VkSemaphoreSubmitInfo signalInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = RenderCompleteSemaphores[SwapchainImageIdx],
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .deviceIndex = 0
        };

        VkSubmitInfo2 submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &waitInfo,
            .commandBufferInfoCount = static_cast<uint32>(cmdBufferInfos.size()),
            .pCommandBufferInfos = cmdBufferInfos.data(),
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signalInfo
        };

        vkQueueSubmit2(GraphicsQueue, 1, &submitInfo, GraphicsFrameFences[frameIdx]);
    }
}

void VulkanDevice::Present(RefCountingPtr<RHIWindow> Window, uint32 SwapchainImageIdx)
{
    if (!Window || !Window->IsValid())
    {
        LE_ASSERT_DESC(false, "Invalid window or window is not valid")
        return;
    }

    VulkanWindow* vulkanWindow = ResourceCast(Window.GetPointer());

    VkPresentInfoKHR presentInfo{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &RenderCompleteSemaphores[SwapchainImageIdx];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = vulkanWindow->GetVkSwapchainPtr();
    presentInfo.pImageIndices = &SwapchainImageIdx;

    CheckVkResult(vkQueuePresentKHR(GraphicsQueue, &presentInfo));
}

RefCountingPtr<RHIPipelineLayout> VulkanDevice::CreatePipelineLayout(const RHIPipelineLayoutDesc& PipelineLayoutDesc)
{
    VkPushConstantRange pushConstantRange{
        .stageFlags = ShaderStage(PipelineLayoutDesc.ShaderStages),
        .size = PipelineLayoutDesc.PushConstantSize
    };

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.reserve(PipelineLayoutDesc.DescriptorSetLayouts.size());
    for (const auto& DescriptorSetLayout : PipelineLayoutDesc.DescriptorSetLayouts)
    {
        descriptorSetLayouts.push_back(ResourceCast(DescriptorSetLayout.GetPointer())->GetVkDescriptorSetLayout());
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange,
    };

    VkPipelineLayout pipelineLayout;
    CheckVkResult(vkCreatePipelineLayout(Device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
    return new VulkanPipelineLayout{pipelineLayout};
}

RefCountingPtr<RHIPipelineObject> VulkanDevice::CreatePipelineObject(const RHIPipelineObjectDesc& PipelineObjectDesc)
{
    VulkanPipelineLayout* PipelineLayout = ResourceCast(PipelineObjectDesc.PipelineLayout.GetPointer());

    VkShaderModuleCreateInfo shaderModuleCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    };
    shaderModuleCreateInfo.codeSize = PipelineObjectDesc.ShaderModule->GetSize();
    shaderModuleCreateInfo.pCode = static_cast<uint32*>(PipelineObjectDesc.ShaderModule->GetData());
    VkShaderModule shaderModule;
    CheckVkResult(vkCreateShaderModule(Device, &shaderModuleCreateInfo, nullptr, &shaderModule));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    auto checkShaderStage = [&shaderStages, &shaderModule, &PipelineObjectDesc](RHIShaderStage stage, const char* entryPoint)
    {
        if (PipelineObjectDesc.Stages & stage)
        {
            VkPipelineShaderStageCreateInfo& shaderStageCI = shaderStages.emplace_back();
            shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCI.stage = ShaderStageFlagBit(stage);
            shaderStageCI.module = shaderModule;
            shaderStageCI.pName = entryPoint;
        }
    };
    checkShaderStage(RHIShaderStage::Vertex, "vertexMain");
    checkShaderStage(RHIShaderStage::Fragment, "fragmentMain");

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    std::vector<VkDynamicState> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState blendAttachment = {};
    blendAttachment.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &blendAttachment;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkFormat swapChainFormat = MapRHIFormat(SwapChainFormat);
    VkFormat depthFormat = MapRHIFormat(DepthFormat);

    VkPipelineRenderingCreateInfo renderingCI = {};
    renderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCI.colorAttachmentCount = 1;
    renderingCI.pColorAttachmentFormats = &swapChainFormat;
    renderingCI.depthAttachmentFormat = depthFormat;

    VkPipelineVertexInputStateCreateInfo emptyInput{};
    emptyInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    emptyInput.vertexBindingDescriptionCount = 0;
    emptyInput.pVertexBindingDescriptions = nullptr;
    emptyInput.vertexAttributeDescriptionCount = 0;
    emptyInput.pVertexAttributeDescriptions = nullptr;

    VkGraphicsPipelineCreateInfo pipelineCI = {};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.pNext = &renderingCI;
    pipelineCI.stageCount = static_cast<uint32>(shaderStages.size());
    pipelineCI.pStages = shaderStages.data();
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pVertexInputState = &emptyInput;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = PipelineLayout->GetVkPipelineLayout();

    VkPipeline pipeline;
    CheckVkResult(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline));

    return new VulkanPipelineObject(pipeline, PipelineObjectDesc.PipelineLayout);
}

void VulkanDevice::DestroyPipelineObject(RefCountingPtr<RHIPipelineObject> PipelineObject)
{
    if (!PipelineObject || !PipelineObject->IsValid())
    {
        return;
    }

    VulkanPipelineObject* pipelineObject = ResourceCast(PipelineObject.GetPointer());

    vkDestroyPipeline(Device, pipelineObject->GetVkPipeline(), nullptr);
}

void VulkanDevice::DestroyPipelineLayout(RefCountingPtr<RHIPipelineLayout> PipelineLayout)
{
    if (!PipelineLayout || !PipelineLayout->IsValid())
    {
        return;
    }

    VulkanPipelineLayout* pipelineLayout = ResourceCast(PipelineLayout.GetPointer());

    vkDestroyPipelineLayout(Device, pipelineLayout->GetVkPipelineLayout(), nullptr);
}

uint64 VulkanDevice::GetCurrentTransferTimelineValue() const
{
    return NextTransferValue.load(std::memory_order_relaxed);
}

RefCountingPtr<RHIWindow> VulkanDevice::CreateWindow(const RHIWindowDesc& WindowDesc)
{
    if (!WindowDesc.NativeWindowHandle)
    {
        LE_ERROR("Invalid native window handle");
        return nullptr;
    }

    if (DepthFormat == RHIFormat::None)
    {
        LE_ERROR("Depth format is not set");
        return nullptr;
    }

    SDL_Window* windowHandle = static_cast<SDL_Window*>(WindowDesc.NativeWindowHandle);

    VkSurfaceKHR windowSurface = VK_NULL_HANDLE;
    CheckVkResult(SDL_Vulkan_CreateSurface(windowHandle, Instance, nullptr, &windowSurface));

    int x = 0;
    int y = 0;
    CheckVkResult(SDL_GetWindowSize(windowHandle, &x, &y));

    VkSurfaceCapabilitiesKHR surfaceCaps = {};
    CheckVkResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, windowSurface, &surfaceCaps));
    VkExtent2D swapchainExtent{surfaceCaps.currentExtent};
    if (surfaceCaps.currentExtent.width == 0xFFFFFFFF)
    {
        swapchainExtent = {.width = static_cast<uint32_t>(x), .height = static_cast<uint32_t>(y)};
    }

    VkSwapchainCreateInfoKHR swapchainCI = {};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.surface = windowSurface;
    swapchainCI.minImageCount = surfaceCaps.minImageCount;
    swapchainCI.imageFormat = MapRHIFormat(SwapChainFormat);
    swapchainCI.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCI.imageExtent = swapchainExtent;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCI.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    CheckVkResult(vkCreateSwapchainKHR(Device, &swapchainCI, nullptr, &swapchain));

    uint32 imageCount = 0;
    CheckVkResult(vkGetSwapchainImagesKHR(Device, swapchain, &imageCount, nullptr));
    std::vector<VkImage> swapchainImages(imageCount);
    CheckVkResult(vkGetSwapchainImagesKHR(Device, swapchain, &imageCount, swapchainImages.data()));

    std::vector<RefCountingPtr<RHIImage>> swapchainImagesRefs(imageCount);
    RHIImageDesc swapchainImageDesc = {};
    swapchainImageDesc.Width = x;
    swapchainImageDesc.Height = y;
    swapchainImageDesc.Depth = 1;
    swapchainImageDesc.MipLevels = 1;
    swapchainImageDesc.ArraySize = 1;
    swapchainImageDesc.Format = SwapChainFormat;
    swapchainImageDesc.Usage = RHIImageUsageFlag::ColorAttachment;

    std::vector<RefCountingPtr<RHIImageView>> swapchainImageViewsRefs(imageCount);
    RHIImageViewDesc genericViewDesc = {};
    genericViewDesc.ViewType = RHIImageViewType::View2D;
    genericViewDesc.Format = SwapChainFormat;
    genericViewDesc.SubresourceRange.Aspect = RHIImageAspectFlags::Color;
    genericViewDesc.SubresourceRange.BaseMipLevel = 0;
    genericViewDesc.SubresourceRange.NumMipLevels = 1;
    genericViewDesc.SubresourceRange.BaseArraySlice = 0;
    genericViewDesc.SubresourceRange.NumArraySlices = 1;

    for (uint32 i = 0; i < imageCount; ++i)
    {
        RefCountingPtr<RHIImage> rhiImage = new VulkanImage(swapchainImageDesc, swapchainImages[i], nullptr);
        swapchainImagesRefs[i] = rhiImage;
        RHIImageViewDesc imageViewDesc = genericViewDesc;
        imageViewDesc.Image = rhiImage;

        swapchainImageViewsRefs[i] = CreateImageView(imageViewDesc);
    }

    RHIImageDesc depthImageCI = {};
    depthImageCI.Format = DepthFormat;
    depthImageCI.Width = x;
    depthImageCI.Height = y;
    depthImageCI.Depth = 1;
    depthImageCI.MipLevels = 1;
    depthImageCI.ArraySize = 1;
    depthImageCI.Usage = RHIImageUsageFlag::DepthStencil;
    RefCountingPtr<RHIImage> depthImage = CreateImage(depthImageCI);

    RHIImageViewDesc depthImageViewCI = {};
    depthImageViewCI.Image = depthImage;
    depthImageViewCI.ViewType = RHIImageViewType::View2D;
    depthImageViewCI.Format = DepthFormat;
    depthImageViewCI.SubresourceRange.Aspect = RHIImageAspectFlags::Depth;
    depthImageViewCI.SubresourceRange.NumMipLevels = 1;
    depthImageViewCI.SubresourceRange.NumArraySlices = 1;
    RefCountingPtr<RHIImageView> depthImageView = CreateImageView(depthImageViewCI);

    return new VulkanWindow(depthImage, depthImageView, swapchainImagesRefs, swapchainImageViewsRefs, SwapChainFormat,
                            x, y,
                            windowSurface,
                            swapchain);
}

void VulkanDevice::DestroyWindow(RefCountingPtr<RHIWindow> Window)
{
    if (!Window || !Window->IsValid())
    {
        return;
    }

    VulkanWindow* vulkanWindow = ResourceCast(Window.GetPointer());
    if (!vulkanWindow)
    {
        return;
    }

    DestroyImage(vulkanWindow->GetDepthImage());
    DestroyImageView(vulkanWindow->GetDepthImageView());

    const uint32 swapchainImageCount = vulkanWindow->GetSwapchainImageCount();
    for (uint32 i = 0; i < swapchainImageCount; ++i)
    {
        DestroyImageView(vulkanWindow->GetSwapchainImageView(i));
    }

    vkDestroySwapchainKHR(Device, vulkanWindow->GetVkSwapchain(), nullptr);
    vkDestroySurfaceKHR(Instance, vulkanWindow->GetVkSurface(), nullptr);
}

bool VulkanDevice::GetNextSwapchainImageIndex(RefCountingPtr<RHIWindow> Window, uint32& OutIndex)
{
    if (!Window || !Window->IsValid())
    {
        return false;
    }

    VulkanWindow* vulkanWindow = ResourceCast(Window.GetPointer());
    const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
    return CheckVkResult(vkAcquireNextImageKHR(Device, vulkanWindow->GetVkSwapchain(), UINT64_MAX,
                                               SwapchainImageAvailableSemaphores[frameIdx], VK_NULL_HANDLE, &OutIndex));
}

RefCountingPtr<RHIImage> VulkanDevice::CreateImage(const RHIImageDesc& ImageDesc)
{
    VkImageCreateInfo imageCI = {};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = GetImageTypeFromImageDesc(ImageDesc);
    imageCI.format = MapRHIFormat(ImageDesc.Format);
    imageCI.extent.width = ImageDesc.Width;
    imageCI.extent.height = ImageDesc.Height;
    imageCI.extent.depth = ImageDesc.Depth;
    imageCI.mipLevels = ImageDesc.MipLevels;
    imageCI.arrayLayers = ImageDesc.ArraySize;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = MapUsageFlags(ImageDesc.Usage);
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocCI{.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO};

    VkImage image = VK_NULL_HANDLE;
    VmaAllocation imageAllocation = nullptr;
    if (!CheckVkResult(vmaCreateImage(Allocator, &imageCI, &allocCI, &image, &imageAllocation, nullptr)))
    {
        return nullptr;
    }

    return new VulkanImage(ImageDesc, image, imageAllocation);
}

void VulkanDevice::DestroyImage(RefCountingPtr<RHIImage> Image)
{
    if (!Image || !Image->IsValid())
    {
        return;
    }

    VulkanImage* vulkanImage = ResourceCast(Image.GetPointer());
    if (!vulkanImage)
    {
        return;
    }

    vmaDestroyImage(Allocator, vulkanImage->GetVkImage(), vulkanImage->GetAllocation());
}

RefCountingPtr<RHIImageView> VulkanDevice::CreateImageView(const RHIImageViewDesc& ImageViewDesc)
{
    VulkanImage* vulkanImage = ResourceCast(ImageViewDesc.Image.GetPointer());
    if (!vulkanImage)
    {
        return nullptr;
    }

    VkImageViewCreateInfo imageViewCI = {};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.image = vulkanImage->GetVkImage();
    imageViewCI.viewType = MapViewType(ImageViewDesc.ViewType);
    imageViewCI.format = MapRHIFormat(ImageViewDesc.Format);
    imageViewCI.subresourceRange = MapSubresourceRange(ImageViewDesc.SubresourceRange);

    VkImageView imageView;
    if (!CheckVkResult(vkCreateImageView(Device, &imageViewCI, nullptr, &imageView)))
    {
        LE_ERROR("Failed to create image view");
        return nullptr;
    }

    return new VulkanImageView(imageView, ImageViewDesc);
}

void VulkanDevice::DestroyImageView(RefCountingPtr<RHIImageView> ImageView)
{
    if (!ImageView || !ImageView->IsValid())
    {
        return;
    }

    VulkanImageView* vulkanImageView = ResourceCast(ImageView.GetPointer());
    if (!vulkanImageView)
    {
        return;
    }

    vkDestroyImageView(Device, vulkanImageView->GetVkImageView(), nullptr);
}

RefCountingPtr<RHISampler> VulkanDevice::CreateSampler(const RHISamplerType& SamplerType)
{
    VkSamplerCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    createInfo.mipLodBias = 0.0f;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = VK_LOD_CLAMP_NONE;
    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    switch (SamplerType)
    {
    case RHISamplerType::LinearRepeat:
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        break;
    case RHISamplerType::LinearClamp:
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        break;
    case RHISamplerType::PointRepeat:
        createInfo.magFilter = VK_FILTER_NEAREST;
        createInfo.minFilter = VK_FILTER_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        break;
    case RHISamplerType::PointClamp:
        createInfo.magFilter = VK_FILTER_NEAREST;
        createInfo.minFilter = VK_FILTER_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        break;
    case RHISamplerType::Anisotropic:
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy = 16.0f;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        break;
    case RHISamplerType::Count:
    default:
        LE_ASSERT_DESC(false, "Invalid sampler type")
        return nullptr;
        break;
    }

    VkSampler sampler;
    if (!CheckVkResult(vkCreateSampler(Device, &createInfo, nullptr, &sampler)))
    {
        LE_ASSERT_DESC(false, "Failed to create sampler")
        return nullptr;
    }

    return new VulkanSampler(sampler);
}

void VulkanDevice::DestroySampler(RefCountingPtr<RHISampler> Sampler)
{
    if (!Sampler || !Sampler->IsValid())
    {
        return;
    }

    VulkanSampler* vulkanSampler = ResourceCast(Sampler.GetPointer());
    if (!vulkanSampler)
    {
        return;
    }

    vkDestroySampler(Device, vulkanSampler->GetVkSampler(), nullptr);
}

RefCountingPtr<RHIDescriptorSetLayout> VulkanDevice::CreateDescriptorSetLayout(const RHIDescriptorSetLayoutDesc& DescriptorSetLayoutDesc)
{
    if (DescriptorSetLayoutDesc.BindingFlags.empty() || DescriptorSetLayoutDesc.Bindings.empty())
    {
        return nullptr;
    }

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<std::vector<VkSampler>> samplers;
    samplers.reserve(DescriptorSetLayoutDesc.Bindings.size());
    bindings.reserve(DescriptorSetLayoutDesc.Bindings.size());
    for (const auto& bindingDesc : DescriptorSetLayoutDesc.Bindings)
    {
        VkDescriptorSetLayoutBinding& bindingCI = bindings.emplace_back();
        bindingCI.binding = bindingDesc.Binding;
        bindingCI.descriptorType = MapDescriptorType(bindingDesc.DescriptorType);
        bindingCI.descriptorCount = bindingDesc.DescriptorCount;
        bindingCI.stageFlags = ShaderStageFlagBit(bindingDesc.ShaderStage);
        if (!bindingDesc.ImmutableSamplers.empty())
        {
            std::vector<VkSampler>& samplerArray = samplers.emplace_back();
            samplerArray.reserve(bindingDesc.ImmutableSamplers.size());
            for (const auto& sampler : bindingDesc.ImmutableSamplers)
            {
                if (!sampler)
                {
                    LE_ASSERT_DESC(false, "Invalid immutable sampler");
                    continue;
                }

                VulkanSampler* vkSampler = ResourceCast(sampler.GetPointer());
                samplerArray.push_back(vkSampler->GetVkSampler());
            }
            bindingCI.pImmutableSamplers = samplerArray.data();
        }
        else
        {
            bindingCI.pImmutableSamplers = nullptr;
        }
    }

    std::vector<VkDescriptorBindingFlags> bindingFlags;
    bindingFlags.reserve(DescriptorSetLayoutDesc.BindingFlags.size());
    for (const auto& bindingFlag : DescriptorSetLayoutDesc.BindingFlags)
    {
        bindingFlags.push_back(MapDescriptorBindingFlags(bindingFlag));
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO
    };
    bindingFlagsInfo.pBindingFlags = bindingFlags.data();
    bindingFlagsInfo.bindingCount = bindingFlags.size();

    VkDescriptorSetLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.pNext = &bindingFlagsInfo;
    layoutInfo.flags = MapDescriptorSetLayoutCreateFlags(DescriptorSetLayoutDesc.Flags);
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout vkDescriptorSetLayout = nullptr;
    if (!CheckVkResult(vkCreateDescriptorSetLayout(Device, &layoutInfo, nullptr, &vkDescriptorSetLayout)))
    {
        return nullptr;
    }

    return new VulkanDescriptorSetLayout(vkDescriptorSetLayout, DescriptorSetLayoutDesc);
}

void VulkanDevice::DestroyDescriptorSetLayout(RefCountingPtr<RHIDescriptorSetLayout> DescriptorSetLayout)
{
    if (!DescriptorSetLayout || !DescriptorSetLayout->IsValid())
    {
        return;
    }

    VulkanDescriptorSetLayout* vulkanLayout = ResourceCast(DescriptorSetLayout.GetPointer());
    vkDestroyDescriptorSetLayout(Device, vulkanLayout->GetVkDescriptorSetLayout(), nullptr);
}

RefCountingPtr<RHIDescriptorSetPool> VulkanDevice::CreateDescriptorSetPool(const RHIDescriptorSetPoolDesc& DescriptorSetPoolDesc)
{
    if (DescriptorSetPoolDesc.PoolSizes.empty())
    {
        return nullptr;
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(DescriptorSetPoolDesc.PoolSizes.size());
    for (const auto& poolSize : DescriptorSetPoolDesc.PoolSizes)
    {
        VkDescriptorPoolSize& vkPoolSize = poolSizes.emplace_back();
        vkPoolSize.type = MapDescriptorType(poolSize.DescriptorType);
        vkPoolSize.descriptorCount = poolSize.DescriptorCount;
    }

    VkDescriptorPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets = DescriptorSetPoolDesc.MaxSets;
    poolInfo.flags = MapDescriptorPoolCreateFlags(DescriptorSetPoolDesc.Flags);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.poolSizeCount = poolSizes.size();

    VkDescriptorPool vkDescriptorPool;
    if (!CheckVkResult(vkCreateDescriptorPool(Device, &poolInfo, nullptr, &vkDescriptorPool)))
    {
        LE_ASSERT_DESC(false, "Failed to create descriptor pool");
        return nullptr;
    }

    return new VulkanDescriptorSetPool(vkDescriptorPool, DescriptorSetPoolDesc);
}

void VulkanDevice::DestroyDescriptorSetPool(RefCountingPtr<RHIDescriptorSetPool> DescriptorSetPool)
{
    if (!DescriptorSetPool || !DescriptorSetPool->IsValid())
    {
        return;
    }

    VulkanDescriptorSetPool* vulkanPool = ResourceCast(DescriptorSetPool.GetPointer());

    vkDestroyDescriptorPool(Device, vulkanPool->GetVkDescriptorSetPool(), nullptr);
}

RefCountingPtr<RHIDescriptorSet> VulkanDevice::CreateDescriptorSet(const RHIDescriptorSetDesc& Desc)
{
    if (!Desc.Layout || !Desc.Pool)
    {
        return nullptr;
    }

    VulkanDescriptorSetPool* vulkanSetPool = ResourceCast(Desc.Pool.GetPointer());
    VulkanDescriptorSetLayout* vulkanSetLayout = ResourceCast(Desc.Layout.GetPointer());

    VkDescriptorSetAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = vulkanSetPool->GetVkDescriptorSetPool();
    allocInfo.pSetLayouts = vulkanSetLayout->GetVkDescriptorSetLayoutPtr();
    allocInfo.descriptorSetCount = 1;

    VkDescriptorSet vkDescriptorSet;
    if (!CheckVkResult(vkAllocateDescriptorSets(Device, &allocInfo, &vkDescriptorSet)))
    {
        LE_ASSERT_DESC(false, "Failed to allocate descriptor set");
        return nullptr;
    }

    return new VulkanDescriptorSet(vkDescriptorSet, Desc.Pool);
}

void VulkanDevice::FreeDescriptorSet(RefCountingPtr<RHIDescriptorSet> DescriptorSet)
{
    if (!DescriptorSet || !DescriptorSet->IsValid())
    {
        return;
    }

    VulkanDescriptorSetPool* vulkanPool = ResourceCast(DescriptorSet->GetPool().GetPointer());
    VulkanDescriptorSet* vulkanDescriptorSet = ResourceCast(DescriptorSet.GetPointer());

    vkFreeDescriptorSets(Device, vulkanPool->GetVkDescriptorSetPool(), 1, vulkanDescriptorSet->GetVkDescriptorSetPtr());
}

void VulkanDevice::UpdateDescriptorSet(const RHIUpdateDescriptorSetDesc& Desc)
{
    if (Desc.ImageInfos.empty() || !Desc.Set || !Desc.Set->IsValid())
    {
        return;
    }

    std::vector<VkDescriptorImageInfo> vulkanImageInfos;
    vulkanImageInfos.reserve(Desc.ImageInfos.size());
    for (const RHIDescriptorImageInfoDesc& imageInfo : Desc.ImageInfos)
    {
        if (!imageInfo.View || !imageInfo.View->IsValid())
        {
            continue;
        }

        VkDescriptorImageInfo& vulkanImageInfo = vulkanImageInfos.emplace_back();
        vulkanImageInfo.imageLayout = MapImageLayout(imageInfo.Layout);
        vulkanImageInfo.imageView = ResourceCast(imageInfo.View.GetPointer())->GetVkImageView();
        if (imageInfo.Sampler && imageInfo.Sampler->IsValid())
        {
            vulkanImageInfo.sampler = ResourceCast(imageInfo.Sampler.GetPointer())->GetVkSampler();
        }
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = ResourceCast(Desc.Set.GetPointer())->GetVkDescriptorSet();
    writeDescriptorSet.dstBinding = Desc.Binding;
    writeDescriptorSet.dstArrayElement = Desc.ArrayElement;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(vulkanImageInfos.size());
    writeDescriptorSet.descriptorType = MapDescriptorType(Desc.DescriptorType);
    writeDescriptorSet.pImageInfo = vulkanImageInfos.data();

    vkUpdateDescriptorSets(Device, 1, &writeDescriptorSet, 0, nullptr);
}

uint32 VulkanDevice::GetGraphicsQueueFamilyIndex() const
{
    return GraphicsQueueFamilyIndex;
}

uint32 VulkanDevice::GetTransferQueueFamilyIndex() const
{
    return TransferQueueFamilyIndex;
}
}
