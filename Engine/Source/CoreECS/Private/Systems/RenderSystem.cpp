#include "Systems/RenderSystem.h"

#include "CoreECSUpdatePasses.h"
#include "EngineGlobals.h"
#include "RendererModule.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "ECS/Ecs.h"
#include "Multithreading/UpdatePasses.h"
#include "SceneRendering/RenderScene.h"
#include "tracy/Tracy.hpp"

namespace LE
{
void RenderSystem::Initialize()
{
	OnAddObserver.ReadsComponents<StaticMeshComponent, TransformComponent>();
	OnAddObserver.AddsResources<Renderer::StaticMeshRenderProxy>();
	OnAddObserver.GetDelegate().Attach<&RenderSystem::OnAdd>(this);
	UpdatePass::AddJob<RenderPass>(&OnAddObserver);

	OnRemoveObserver.DeletesResources<Renderer::StaticMeshRenderProxy>();
	OnRemoveObserver.GetDelegate().Attach<&RenderSystem::OnRemove>(this);
	UpdatePass::AddJob<RenderPass>(&OnRemoveObserver);

	RenderUpdateStaticMesh.GetDelegate().Attach<&RenderSystem::UpdateStaticMeshes>(this);
	RenderUpdateStaticMesh.ReadsComponents<StaticMeshComponent, TransformComponent>();
	RenderUpdateStaticMesh.ReadsResources<Renderer::StaticMeshRenderProxy>();
	UpdatePass::AddJob<RenderPass>(&RenderUpdateStaticMesh);

	RenderUpdateCamera.GetDelegate().Attach<&RenderSystem::UpdateCamera>(this);
	RenderUpdateCamera.ReadsComponents<CameraComponent, TransformComponent>();
	UpdatePass::AddJob<RenderPass>(&RenderUpdateCamera);
}

void RenderSystem::Shutdown()
{
}

void RenderSystem::UpdateStaticMeshes(const float DeltaSeconds)
{
	ZoneScopedN("RenderSystem::UpdateStaticMeshes");
	Renderer::RenderScene& renderScene = GetRendererModule()->GetRenderScene();
	auto view = ViewComponents<StaticMeshComponent, TransformComponent>();
	for (const EcsEntity& entity : view)
	{
		const TransformComponent& transformComponent = view.GetComponents<TransformComponent>(entity);
		renderScene.UpdateStaticMeshProxyTransform(entity, transformComponent.Transform);
	}
}

void RenderSystem::UpdateCamera(const float DeltaSeconds)
{
	ZoneScopedN("RenderSystem::UpdateCamera");
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

void RenderSystem::OnAdd(const OnAddObserverType::ObserverType& Observer)
{
	ZoneScopedN("RenderSystem::OnAdd");
	Renderer::RenderScene& renderScene = GetRendererModule()->GetRenderScene();
	for (auto entity : Observer)
	{
		const StaticMeshComponent& staticMeshComponent = Observer.GetComponents<StaticMeshComponent>(entity);
		const TransformComponent& transformComponent = Observer.GetComponents<TransformComponent>(entity);

		renderScene.CreateStaticMeshRenderProxy(entity, transformComponent.Transform, staticMeshComponent.RenderData, staticMeshComponent.MeshMaterial);
	}
}

void RenderSystem::OnRemove(const OnRemoveObserverType::ObserverType& Observer)
{
	ZoneScopedN("RenderSystem::OnRemove");
	Renderer::RenderScene& renderScene = GetRendererModule()->GetRenderScene();
	for (auto entity : Observer)
	{
		renderScene.DeleteRenderObjectProxy(entity);
	}
}
}
