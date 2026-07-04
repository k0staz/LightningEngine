#pragma once
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "Templates/NonCopyable.h"

namespace LE::RHI
{
class RHIDevice : public NonCopyable
{
public:
	RHIDevice(RHIDeviceType DeviceType)
		: NonCopyable(),
		  CurrentDeviceType(DeviceType)
	{}

	static RHIDevice* Get();
	static void Register(RHIDevice* DeviceImplementation);

	virtual ~RHIDevice() = default;

	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual void WaitIdle() = 0;

	RHIDeviceType GetDeviceType() const { return CurrentDeviceType; }

	/**
	 * @brief Blocks the CPU until the earliest frame-in-flight completes on the GPU, then resets command pools and resource tracking for that frame index.
	 */
	virtual void BeginFrame() = 0;

	virtual uint32 GetGraphicsQueueFamilyIndex() const { return 0; }
	virtual uint32 GetTransferQueueFamilyIndex() const { return 0; }

	virtual RefCountingPtr<RHIBuffer> CreateBuffer(RHIBufferDescription BufferDesc) = 0;
	virtual void DestroyBuffer(RefCountingPtr<RHIBuffer> Buffer) = 0;

	/**
	 * @brief Creates command list. Thread Safe
	 * @param ListType Type of queue for which the command list should be created
	 * @return Created RHI Command list, user is responsible for starting recording
	 */
	virtual RefCountingPtr<RHICommandList> CreateCommandList(RHICommandListType ListType) = 0;

	/**
	 * @brief Submits command lists to the queue. Not thread safe, should be used from Render Thread Only.
	 * @param ListType Type of queue for which the command list should be created
	 * @param CommandLists Command lists of the same type, which will be submitted to the queue of that type
	 * @param SwapchainImageIdx
	 */
	virtual void SubmitCommandList(RHICommandListType ListType, const std::vector<RefCountingPtr<RHICommandList>>& CommandLists, uint32 SwapchainImageIdx = 0) = 0;

	virtual void Present(RefCountingPtr<RHIWindow> Window, uint32 SwapchainImageIdx) = 0;
	
	virtual RefCountingPtr<RHIPipelineLayout> CreatePipelineLayout(const RHIPipelineLayoutDesc& PipelineLayoutDesc) = 0;
	virtual RefCountingPtr<RHIPipelineObject> CreatePipelineObject(const RHIPipelineObjectDesc& PipelineObjectDesc) = 0;

	virtual void DestroyPipelineObject(RefCountingPtr<RHIPipelineObject> PipelineObject) = 0;
	virtual void DestroyPipelineLayout(RefCountingPtr<RHIPipelineLayout> PipelineLayout) = 0;
	
	virtual RefCountingPtr<RHIWindow> CreateWindow(const RHIWindowDesc& WindowDesc) = 0;
	virtual void DestroyWindow(RefCountingPtr<RHIWindow> Window) = 0;

	virtual bool GetNextSwapchainImageIndex(RefCountingPtr<RHIWindow> Window, uint32& OutIndex) = 0;

	virtual RefCountingPtr<RHIImage> CreateImage(const RHIImageDesc& ImageDesc) = 0;
	virtual void DestroyImage(RefCountingPtr<RHIImage> Image) = 0;

	virtual RefCountingPtr<RHIImageView> CreateImageView(const RHIImageViewDesc& ImageViewDesc) = 0;
	virtual void DestroyImageView(RefCountingPtr<RHIImageView> ImageView) = 0;

	virtual uint64 GetCurrentTransferTimelineValue() const = 0;

	virtual RefCountingPtr<RHISampler> CreateSampler(const RHISamplerType& SamplerDesc) = 0;
	virtual void DestroySampler(RefCountingPtr<RHISampler> Sampler) = 0;

	virtual RefCountingPtr<RHIDescriptorSetLayout> CreateDescriptorSetLayout(const RHIDescriptorSetLayoutDesc& DescriptorSetLayoutDesc) = 0;
	virtual void DestroyDescriptorSetLayout(RefCountingPtr<RHIDescriptorSetLayout> DescriptorSetLayout) = 0;

	virtual RefCountingPtr<RHIDescriptorSetPool> CreateDescriptorSetPool(const RHIDescriptorSetPoolDesc& DescriptorSetPoolDesc) = 0;
	virtual void DestroyDescriptorSetPool(RefCountingPtr<RHIDescriptorSetPool> DescriptorSetPool) = 0;

	virtual RefCountingPtr<RHIDescriptorSet> CreateDescriptorSet(const RHIDescriptorSetDesc& Desc) = 0;
	virtual void FreeDescriptorSet(RefCountingPtr<RHIDescriptorSet> DescriptorSet) = 0;

	virtual void UpdateDescriptorSet(const RHIUpdateDescriptorSetDesc& Desc) = 0;

protected:
	RHIDeviceType CurrentDeviceType = RHIDeviceType::None;
};
}
