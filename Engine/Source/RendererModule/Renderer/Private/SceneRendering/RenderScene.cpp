#include "SceneRendering/RenderScene.h"

#include "RenderCommandList.h"
#include "RenderContributors/RenderContributorManager.h"
#include "RenderContributors/MeshContributors/StaticMeshContributor.h"
#include "RenderResourceManager/RenderResourceManager.h"

namespace LE::Renderer
{
RenderScene::~RenderScene()
{
	RenderProxies.Clear();
}

bool RenderScene::HasRenderProxy(EcsEntity EntityId) const
{
	return RenderProxies.Has(EntityId);
}

void RenderScene::CreateStaticMeshRenderProxy(EcsEntity EntityId, AssetHandle<StaticMeshAsset> MeshAsset, const Matrix4x4F& Transform)
{
	RenderCommandList::Get().EnqueueLambdaCommand([this, EntityId, MeshAsset, Transform](RenderCommandList& CmdList)
	{
		if (RenderProxies.Has(EntityId))
		{
			return;
		}

		RenderProxyState& state = RenderProxies.AddProxy(EntityId);
		state.EntityId = EntityId;
		state.IsEnabled = true;
		
		RenderResourceManager& resourceManager = GetServiceRegistry().GetService<RenderResourceManager>();
		auto resourceHandle = resourceManager.RequestStaticMesh(MeshAsset);
		RenderContributorId contributorId = resourceManager.GetRenderContributor(resourceHandle);

		state.MeshVariationTypeId = RenderContributorTypeIdGetter<StaticMeshContributor>::Value;
		state.MeshVariationInstanceId = contributorId;

		RenderContributorManager& contributorManager = GetServiceRegistry().GetService<RenderContributorManager>();
		StaticMeshContributor& contributor = contributorManager.GetRenderContributor<StaticMeshContributor>(contributorId);

		contributor.AddProxy(EntityId);
		contributor.GetDynamicData(EntityId).LocalToWorld = Transform;
	});
}

void RenderScene::UpdateStaticMeshRenderProxy(EcsEntity EntityId, const Matrix4x4F& Transform)
{
	RenderCommandList::Get().EnqueueLambdaCommand([this, EntityId, Transform](RenderCommandList& CmdList)
	{
		if (!RenderProxies.Has(EntityId))
		{
			return;
		}

		RenderContributorManager& contributorManager = GetServiceRegistry().GetService<RenderContributorManager>();
		StaticMeshContributor& contributor = contributorManager.GetRenderContributor<StaticMeshContributor>(
			RenderProxies.GetProxyState(EntityId).MeshVariationInstanceId);
		contributor.GetDynamicData(EntityId).LocalToWorld = Transform;
	}
	);
}

void RenderScene::DeleteRenderProxy(EcsEntity EntityId)
{
	RenderCommandList::Get().EnqueueLambdaCommand([this, EntityId](RenderCommandList& CmdList)
	{
		if (!RenderProxies.Has(EntityId))
		{
			return;
		}

		RenderProxyState& proxyState = RenderProxies.GetProxyState(EntityId);
		RenderContributorManager& contributorManager = GetServiceRegistry().GetService<RenderContributorManager>();
		RenderContributor& contributor = contributorManager.GetRenderContributor(proxyState.MeshVariationTypeId, proxyState.MeshVariationInstanceId);
		contributor.RemoveProxy(EntityId);
	});
}

void RenderScene::SetRenderProxyEnabled(EcsEntity EntityId, bool Enabled)
{
	RenderCommandList::Get().EnqueueLambdaCommand([this, EntityId, Enabled](RenderCommandList& CmdList)
	{
		if (!RenderProxies.Has(EntityId))
		{
			return;
		}

		RenderProxyState& proxyState = RenderProxies.GetProxyState(EntityId);
		proxyState.IsEnabled = Enabled;
	});

}

void RenderScene::GetEnabledRenderProxies(std::vector<RenderProxyState*>& EnabledProxies)
{
	EnabledProxies.clear();
	EnabledProxies.reserve(RenderProxies.Capacity());
	for (EcsEntity EntityId : RenderProxies)
	{
		RenderProxyState& proxyState = RenderProxies.GetProxyState(EntityId);
		if (proxyState.IsEnabled)
		{
			EnabledProxies.push_back(&proxyState);
		}
	}
}
}
