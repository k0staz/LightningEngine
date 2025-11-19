#include "AssetManager/Asset.h"

#include "AssetManager/AssetManager.h"

namespace LE
{
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
}
