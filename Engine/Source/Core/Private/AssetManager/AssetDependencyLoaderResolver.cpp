#include "AssetManager/AssetDependencyLoaderResolver.h"

#include "AssetManager/AssetManager.h"
#include "AssetManager/AssetRegistry.h"
#include "Multithreading/JobScheduler.h"
#include "Service/ServiceRegistry.h"
#include "tracy/Tracy.hpp"

namespace LE
{
void AssetDependencyLoaderResolver::LoadAsset(const Uid& AssetUid)
{
	AssetRegistry& registry = GetServiceRegistry().GetService<AssetRegistry>();
	AssetManager& assetManager = GetServiceRegistry().GetService<AssetManager>();

	AssetHandle<> loadingAsset = assetManager.GetAsset(AssetUid);
	if(loadingAsset->IsLoaded())
	{
		return;
	}

	if(!loadingAsset->TrySetLoadingState())
	{
		loadingAsset->WaitUntilLoaded();
		return;
	}

	AssetInfo assetInfo = registry.GetAssetInfo(AssetUid);

	// Iterate over dependencies and load them
	for (const Uid& dependency : assetInfo.Dependencies)
	{
		LoadAsset(dependency);
		AssetHandle<> assetHandle = assetManager.GetAsset(dependency);
		assetHandle->AddLoadingRef();
	}

	assetManager.InternalLoadAsset(assetInfo);
	RemoveLoadingRefs(assetInfo);
}

void AssetDependencyLoaderResolver::LoadAssetAsync(const Uid& AssetUid)
{
	AssetRegistry& registry = GetServiceRegistry().GetService<AssetRegistry>();
	AssetManager& assetManager = GetServiceRegistry().GetService<AssetManager>();

	AssetHandle<> loadingAsset = assetManager.GetAsset(AssetUid);
	if(loadingAsset->IsLoaded() || loadingAsset->IsLoading())
	{
		return;
	}

	if(!loadingAsset->TrySetLoadingState())
	{
		return;
	}

	AssetInfo& assetInfo = registry.GetAssetInfo(AssetUid);
	auto loadingFunction = [assetInfo]() mutable 
	{
		ZoneScopedNC("LoadingTask::Asset", tracy::Color::Purple);
		AssetManager& assetManager = GetServiceRegistry().GetService<AssetManager>();
		assetManager.InternalLoadAsset(assetInfo);
		RemoveLoadingRefs(assetInfo);
	};

	JobScheduler& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();
	RefCountingPtr<AsyncTaskNodeBase> loadingTaskNode = new AsyncTaskNode<void()>{"LoadingTask", &jobScheduler, loadingFunction};

	LE_ASSERT_DESC(!assetInfo.LoadingTask, "Overwriting existing Loading Task, something is wrong")
	assetInfo.LoadingTask = loadingTaskNode;
	
	for (const Uid& dependency : assetInfo.Dependencies)
	{
		LoadAssetAsync(dependency);
		AssetHandle<> assetHandle = assetManager.GetAsset(dependency);
		assetHandle->AddLoadingRef();

		AssetInfo dependencyInfo = registry.GetAssetInfo(dependency);
		if(!dependencyInfo.LoadingTask)
		{
			LE_ASSERT_DESC(false, "Missing loading task")
			continue;
		}
		dependencyInfo.LoadingTask->AddDependentTask(loadingTaskNode);
	}

	loadingTaskNode->Finalize();
}

void AssetDependencyLoaderResolver::RemoveLoadingRefs(const AssetInfo& Info)
{
	AssetManager& assetManager = GetServiceRegistry().GetService<AssetManager>();
	for (const Uid& dependency : Info.Dependencies)
	{
		AssetHandle<> assetHandle = assetManager.GetAsset(dependency);
		assetHandle->ReleaseLoading();
	}
}
}
