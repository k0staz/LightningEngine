#include "RenderResourceManager/RenderResources.h"

#include "RHIDevice.h"

namespace LE::Renderer
{
bool RenderResource::IsLoaded() const
{
	const RenderResourceState state = GetState();
	if(state == RenderResourceState::Ready)
	{
		return true;
	}
	else if (state == RenderResourceState::Unloaded)
	{
		return false;
	}
	
	const uint64 currentTransferValue = RHI::RHIDevice::Get()->GetCurrentTransferTimelineValue();
	if (currentTransferValue > BatchTransferValue)
	{
		SetState(RenderResourceState::Ready);
		return true;
	}
	
	return false;
}

uint32 RenderResource::AddRef() const
{
	uint32 newValue = RefsNum.fetch_add(1, std::memory_order_relaxed) + 1;
	return newValue;
}

uint32 RenderResource::ReleaseRef() const
{
	uint32 newValue = RefsNum.fetch_sub(1, std::memory_order_acq_rel) - 1;
	return newValue;
}

void RenderResource::SetState(RenderResourceState NewState) const
{
	State.store(NewState, std::memory_order_relaxed);
	State.notify_all();
}

bool RenderResource::TrySetLoadingState()
{
	RenderResourceState expected = RenderResourceState::Unloaded;
	State.compare_exchange_strong(expected, RenderResourceState::Loading);

	return expected == RenderResourceState::Unloaded;
}
}
