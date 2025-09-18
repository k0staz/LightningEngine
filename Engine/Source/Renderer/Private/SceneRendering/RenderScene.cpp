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

void RenderScene::CreateStaticMeshRenderProxy(EcsEntity Entity, const Matrix4x4F& Transform, const StaticMeshRenderData* RenderData,
                                              const Material* MeshMaterial)
{
	// TODO: Here we need to create material instance based on the data from component
	MaterialInstance* materialInstance = new MaterialInstance(MeshMaterial);
	Vector4F color = {1.0f, 0.0f, 0.0f, 1.0f};
	materialInstance->SetParameter(&BasePS::StaticGetMetaType(), "Color", reinterpret_cast<uint8*>(&color));
	StaticMeshRenderProxy* newProxy = new StaticMeshRenderProxy(Entity, RenderData, materialInstance);

	// Handle over to render thread
	RenderCommandList::Get().EnqueueLambdaCommand([this, newProxy, Entity, Transform](RenderCommandList& CmdList)
	{
		if (RenderObjectProxies.contains(Entity))
		{
			LE_ASSERT_DESC(false, "[Render Scene] Render Proxy double Add")
			delete newProxy;
			return;
		}

		RenderObjectProxies[Entity] = newProxy;
		newProxy->CreateConstantBuffer();
		newProxy->SetTransform(Transform);
	});
}

void RenderScene::DeleteRenderObjectProxy(EcsEntity Entity)
{
	RenderCommandList::Get().EnqueueLambdaCommand([this, Entity](RenderCommandList& CmdList)
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
	});
}

void RenderScene::UpdateStaticMeshProxyTransform(EcsEntity Entity, const Matrix4x4F& Transform)
{
	RenderCommandList::Get().EnqueueLambdaCommand([this, Transform, Entity](RenderCommandList& CmdList)
	{
		if (!RenderObjectProxies.contains(Entity))
		{
			return;
		}

		RenderObjectProxy* proxy = RenderObjectProxies[Entity];
		proxy->SetTransform(Transform);
		proxy->UpdateConstantBuffer(CmdList);
	});
}
}
