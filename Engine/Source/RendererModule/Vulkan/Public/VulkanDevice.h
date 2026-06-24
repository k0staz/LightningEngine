#pragma once

#include <array>

#include "RHIDevice.h"
#include "VulkanFwd.h"
#include "VulkanThreadResources.h"

namespace LE::RHI::Vulkan
{
class VulkanDevice : public RHIDevice
{
public:
	VulkanDevice();

	void Initialize() override;
	void Shutdown() override;
	void WaitIdle() override;

	void BeginFrame() override;

	RefCountingPtr<RHIBuffer> CreateBuffer(RHIBufferDescription BufferDesc) override;
	void DestroyBuffer(RefCountingPtr<RHIBuffer> Buffer) override;
	void CopyToGlobalBuffer(RefCountingPtr<RHICommandList> CommandList, RefCountingPtr<RHIGlobalBuffer> GlobalBuffer,
	                        RefCountingPtr<RHILinearBuffer> StageBuffer, const std::vector<RHIGlobalBufferUploadDesc>& Descriptions) override;

	RefCountingPtr<RHICommandList> CreateCommandList(RHICommandListType ListType) override;
	void SubmitCommandList(RHICommandListType ListType, const std::vector<RefCountingPtr<RHICommandList>>& CommandLists, uint32 SwapchainImageIdx) override;

	void Present(RefCountingPtr<RHIWindow> Window, uint32 SwapchainImageIdx) override;

	RefCountingPtr<RHIPipelineLayout> CreatePipelineLayout(const RHIPipelineLayoutDesc& PipelineLayoutDesc) override;
	RefCountingPtr<RHIPipelineObject> CreatePipelineObject(const RHIPipelineObjectDesc& PipelineObjectDesc) override;

	void DestroyPipelineObject(RefCountingPtr<RHIPipelineObject> PipelineObject) override;
	void DestroyPipelineLayout(RefCountingPtr<RHIPipelineLayout> PipelineLayout) override;

	uint64 GetCurrentTransferTimelineValue() const override;

	RefCountingPtr<RHIWindow> CreateWindow(const RHIWindowDesc& WindowDesc) override;
	void DestroyWindow(RefCountingPtr<RHIWindow> Window) override;

	bool GetNextSwapchainImageIndex(RefCountingPtr<RHIWindow> Window, uint32& OutIndex) override;

	RefCountingPtr<RHIImage> CreateImage(const RHIImageDesc& ImageDesc) override;
	void DestroyImage(RefCountingPtr<RHIImage> Image) override;

	RefCountingPtr<RHIImageView> CreateImageView(const RHIImageViewDesc& ImageViewDesc) override;
	void DestroyImageView(RefCountingPtr<RHIImageView> ImageView) override;

private:
	VkInstance Instance = nullptr;
	VkDevice Device = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
	VkQueue GraphicsQueue = nullptr;
	VkFence GraphicsFrameFences[DEFAULT_FRAMES_IN_FLIGHT];
	std::array<VkSemaphore, DEFAULT_FRAMES_IN_FLIGHT> RenderCompleteSemaphores;
	std::array<VkSemaphore, DEFAULT_FRAMES_IN_FLIGHT> SwapchainImageAvailableSemaphores;
	VkQueue TransferQueue = nullptr;
	VulkanThreadResources GraphicsThreadResources[DEFAULT_TASK_WORKER_THREADS + 1][DEFAULT_FRAMES_IN_FLIGHT];
	VulkanThreadResources TransferThreadResources[DEFAULT_TASK_WORKER_THREADS + 1][DEFAULT_FRAMES_IN_FLIGHT];
	VkSemaphore TransferTimelineSemaphore = nullptr;
	std::atomic<uint64> NextTransferValue{1};
	uint64 TransferFrameSemaphoreValues[DEFAULT_FRAMES_IN_FLIGHT];

	RHIFormat DepthFormat;
	RHIFormat SwapChainFormat;

	VmaAllocator Allocator = nullptr;

};
}
