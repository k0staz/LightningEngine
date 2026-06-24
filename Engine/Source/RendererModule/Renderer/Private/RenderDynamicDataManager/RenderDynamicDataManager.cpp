#include "RenderDynamicDataManager/RenderDynamicDataManager.h"

#include "RenderCore.h"
#include "RenderDefines.h"
#include "RHIDevice.h"

namespace LE::Renderer
{
void RenderDynamicDataManager::Initialize()
{
	RHI::RHIDevice* device = RHI::RHIDevice::Get();
	for (uint32 i = 0; i < DEFAULT_FRAMES_IN_FLIGHT; ++i)
	{
		RHI::RHIBufferDescription frameDataBufferDescription;
		frameDataBufferDescription.UsageType = RHI::RHIBufferUsageType::DynamicFrameData;
		frameDataBufferDescription.Size = GlobalFrameDataBufferSize;
		FrameRingBuffer[i] = device->CreateBuffer(frameDataBufferDescription);
	}
}

void RenderDynamicDataManager::Shutdown()
{
	RHI::RHIDevice* device = RHI::RHIDevice::Get();
	for (uint32 i = 0; i < DEFAULT_FRAMES_IN_FLIGHT; ++i)
	{
		device->DestroyBuffer(FrameRingBuffer[i]);
	}
}

void RenderDynamicDataManager::OnBeginFrame() const
{
	const uint64 frameIdx = Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT;
	FrameRingBuffer[frameIdx]->ResetBufferToDefault();
}

RefCountingPtr<RHI::RHILinearBuffer> RenderDynamicDataManager::GetCurrentFrameRingBuffer() const
{
	return FrameRingBuffer[Renderer::GetCurrentRenderFrame() % DEFAULT_FRAMES_IN_FLIGHT];
}
}
