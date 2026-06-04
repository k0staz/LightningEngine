#include "AssetManager/Asset.h"

#include "AssetManager/AssetManager.h"

namespace LE
{
void Asset::WaitUntilLoaded() const
{
	State.wait(AssetState::Loading, std::memory_order_consume);
}

void Asset::SetState(AssetState NewState) const
{
	State.store(NewState, std::memory_order_relaxed);
	State.notify_all();
}

bool Asset::TrySetLoadingState() const
{
	AssetState expected = AssetState::Uninitialized;
	State.compare_exchange_strong(expected, AssetState::Loading);

	return expected == AssetState::Uninitialized;
}

uint32 Asset::AddRef() const
{
	uint32 newValue = RefsNum.fetch_add(1, std::memory_order_relaxed) + 1;
	return newValue;
}

uint32 Asset::Release() const
{
	uint32 newValue = RefsNum.fetch_sub(1, std::memory_order_acq_rel) - 1;
	return newValue;
}

uint32 Asset::AddLoadingRef() const
{
	uint32 newValue = LoadingRefsNum.fetch_add(1, std::memory_order_relaxed) + 1;
	return newValue;
}

uint32 Asset::ReleaseLoading() const
{
	uint32 newValue = LoadingRefsNum.fetch_sub(1, std::memory_order_acq_rel) - 1;
	return newValue;
}
}
