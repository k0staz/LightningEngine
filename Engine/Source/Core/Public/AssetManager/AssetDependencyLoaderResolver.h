#pragma once
#include "Misc/Uid.h"

namespace LE
{
// Loads assets in the dependency order
class AssetDependencyLoaderResolver
{
public:
	// Loads assets linearly
	void LoadAsset(const Uid& AssetUid);

	// Creates loading Tasks and schedules them in the dependency order
	void LoadAssetAsync(const Uid& AssetUid);
private:
	static void RemoveLoadingRefs(const AssetInfo& Info);
};
}
