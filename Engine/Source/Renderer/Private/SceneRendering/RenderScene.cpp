#include "SceneRendering/RenderScene.h"

#include "RenderPasses/BaseRenderPass.h"

namespace LE::Renderer
{
RenderScene::~RenderScene()
{
	for (auto& proxyEntry : RenderObjectProxies)
	{
		RenderObjectProxy* proxy = proxyEntry.second;
		if (!proxy)
		{
			continue;
		}

		delete proxy;
		proxy = nullptr;
	}

	RenderObjectProxies.clear();
}

StaticMeshRenderProxy* RenderScene::CreateStaticMeshRenderProxy(EcsEntity Entity, const StaticMeshRenderData* RenderData, const Material* MeshMaterial)
{
	if (RenderObjectProxies.contains(Entity))
	{
		LE_ASSERT_DESC(false, "[Render Scene] Render Proxy double Add")
		return dynamic_cast<StaticMeshRenderProxy*>(RenderObjectProxies[Entity]);
	}

	// TODO: Here we need to create material instance based on the data from component
	MaterialInstance* materialInstance = new MaterialInstance(MeshMaterial);
	Vector4F color = { 1.0f, 0.0f, 0.0f, 1.0f };
	materialInstance->SetParameter(&BasePS::StaticGetMetaType(), "Color", reinterpret_cast<uint8*>(&color));
	StaticMeshRenderProxy* newProxy = new StaticMeshRenderProxy(Entity, RenderData, materialInstance);
	RenderObjectProxies[Entity] = newProxy;

	return newProxy;
}

void RenderScene::DeleteRenderObjectProxy(EcsEntity Entity)
{
	if (!RenderObjectProxies.contains(Entity))
	{
		LE_WARN("Trying to delete proxy, which container doesn't have for entity %d", Entity);
		return;
	}

	RenderObjectProxy* proxy = RenderObjectProxies[Entity];
	delete proxy;
	proxy = nullptr;

	RenderObjectProxies.erase(Entity);
}
}
