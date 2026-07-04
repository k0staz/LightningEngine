#pragma once

// Forward declarations for Vulkan types
struct VkInstance_T;
typedef VkInstance_T* VkInstance;

struct VkPhysicalDevice_T;
typedef VkPhysicalDevice_T* VkPhysicalDevice;

struct VkDevice_T;
typedef VkDevice_T* VkDevice;

struct VkQueue_T;
typedef VkQueue_T* VkQueue;

struct VkBuffer_T;
typedef VkBuffer_T* VkBuffer;

struct VkCommandPool_T;
typedef VkCommandPool_T* VkCommandPool;

struct VkCommandBuffer_T;
typedef VkCommandBuffer_T* VkCommandBuffer;

struct VkSemaphore_T;
typedef VkSemaphore_T* VkSemaphore;

struct VkFence_T;
typedef VkFence_T* VkFence;

struct VkPipelineLayout_T;
typedef VkPipelineLayout_T* VkPipelineLayout;

struct VkPipeline_T;
typedef VkPipeline_T* VkPipeline;

struct VkSurfaceKHR_T;
typedef VkSurfaceKHR_T* VkSurfaceKHR;

struct VkSwapchainKHR_T;
typedef VkSwapchainKHR_T* VkSwapchainKHR;

struct VkImage_T;
typedef VkImage_T* VkImage;

struct VkImageView_T;
typedef VkImageView_T* VkImageView;

struct VkSampler_T;
typedef VkSampler_T* VkSampler;

struct VkDescriptorSetLayout_T;
typedef VkDescriptorSetLayout_T* VkDescriptorSetLayout;

struct VkDescriptorPool_T;
typedef VkDescriptorPool_T* VkDescriptorPool;

struct VkDescriptorSet_T;
typedef VkDescriptorSet_T* VkDescriptorSet;

typedef uint32_t VkShaderStageFlags; 

typedef uint64_t VkDeviceAddress;

// Forward declaration for VMA
struct VmaAllocator_T;
typedef VmaAllocator_T* VmaAllocator;

struct VmaAllocation_T;
typedef VmaAllocation_T* VmaAllocation;

struct VmaVirtualBlock_T;
typedef VmaVirtualBlock_T* VmaVirtualBlock;

struct VmaVirtualAllocation_T;
typedef VmaVirtualAllocation_T* VmaVirtualAllocation;