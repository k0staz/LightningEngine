#include "Systems/RenderSystem.h"

#include "EngineGlobals.h"
#include "RendererModule.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "ECS/Ecs.h"
#include "SceneRendering/RenderScene.h"

namespace LE
{
void RenderSystem::Initialize()
{
	OnAddObserver = ObserverComponents<StaticMeshComponent, TransformComponent>(ComponentChangeType::ComponentAdded);
	OnRemoveObserver = ObserverComponents<StaticMeshComponent, TransformComponent>(ComponentChangeType::ComponentRemoved);
}

void RenderSystem::Update(const float DeltaSeconds)
{
	Renderer::RenderScene& renderScene = GetRendererModule()->GetRenderScene();
	for (auto entity : OnAddObserver)
	{
		const StaticMeshComponent& staticMeshComponent = OnAddObserver.GetComponents<StaticMeshComponent>(entity);
		const TransformComponent& transformComponent = OnAddObserver.GetComponents<TransformComponent>(entity);

		Renderer::StaticMeshRenderProxy* proxy = renderScene.CreateStaticMeshRenderProxy(entity, staticMeshComponent.RenderData, staticMeshComponent.MeshMaterial);
		if (!proxy)
		{
			LE_ASSERT_DESC(false, "Failed to create static mesh render proxy for entity {}", entity)
			continue;
		}

		proxy->SetTransform(transformComponent.Transform);
		proxy->CreateConstantBuffer();
	}
	OnAddObserver.ResetObservedEntities();

	for (auto entity : OnRemoveObserver)
	{
		renderScene.DeleteRenderObjectProxy(entity);
	}
	OnRemoveObserver.ResetObservedEntities();

	const auto& proxyMap = renderScene.GetProxyMap();

	auto view = ViewComponents<StaticMeshComponent, TransformComponent>();
	for (const EcsEntity& entity : view)
	{
		const TransformComponent& transformComponent = view.GetComponents<TransformComponent>(entity);

		if (!proxyMap.contains(entity))
		{
			continue;
		}

		Renderer::RenderObjectProxy* proxy = proxyMap.at(entity);
		proxy->SetTransform(transformComponent.Transform);
		proxy->UpdateConstantBuffer(Renderer::RenderCommandList::Get());
	}

	auto cameraView = ViewComponents<CameraComponent, TransformComponent>();
	for (const EcsEntity& entity : cameraView)
	{
		// TODO: For now we always assume that we have one camera, later we will need to handle multiple active cameras out of all cameras
		const TransformComponent& transformComponent = cameraView.GetComponents<TransformComponent>(entity);
		const CameraComponent& cameraComponent = cameraView.GetComponents<CameraComponent>(entity);

		Renderer::SceneViewInfo newViewInfo;
		newViewInfo.FOV = cameraComponent.FOV;
		newViewInfo.ViewTransform = transformComponent.Transform;
		GetWorld()->SetPrimaryViewInfo(newViewInfo);
	}
}

void RenderSystem::Shutdown()
{
}
}
