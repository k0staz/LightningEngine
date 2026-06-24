#include "RenderSystem.h"

#include "RendererECSUpdatePasses.h"
#include "EngineGlobals.h"
#include "ModuleRegistry.h"
#include "RendererModule.h"
#include "CameraComponent.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"
#include "ECS/Ecs.h"
#include "Multithreading/UpdatePasses.h"
#include "SceneRendering/RenderScene.h"
#include "SceneRendering/SceneView.h"
#include "tracy/Tracy.hpp"

namespace LE
{
void RenderSystem::Initialize()
{
	OnAddObserver.ReadsComponents<TransformComponent>();
	OnAddObserver.WritesComponents<StaticMeshComponent>();
	OnAddObserver.GetDelegate().Attach<&RenderSystem::OnAdd>(this);
	UpdatePass::AddJob<RenderPass>(&OnAddObserver);
	
	OnRemoveObserver.GetDelegate().Attach<&RenderSystem::OnRemove>(this);
	UpdatePass::AddJob<RenderPass>(&OnRemoveObserver);

	RenderUpdateStaticMesh.GetDelegate().Attach<&RenderSystem::UpdateStaticMeshes>(this);
	RenderUpdateStaticMesh.ReadsComponents<TransformComponent>();
	RenderUpdateStaticMesh.WritesComponents<StaticMeshComponent>();
	UpdatePass::AddJob<RenderPass>(&RenderUpdateStaticMesh);

	RenderUpdateCamera.GetDelegate().Attach<&RenderSystem::UpdateCamera>(this);
	RenderUpdateCamera.ReadsComponents<CameraComponent, TransformComponent>();
	UpdatePass::AddJob<RenderPass>(&RenderUpdateCamera);
}

void RenderSystem::Shutdown()
{
}
// TODO: should update only changed components
void RenderSystem::UpdateStaticMeshes(const float DeltaSeconds)
{
	ZoneScopedN("RenderSystem::UpdateStaticMeshes");
	//TODO: This needs to be reworked, we should not be allowed to get any component we want within job observer
	// Instead we should take in observer payload, which allows to take either const ref, or ref
	// Each usage of simple ref should mark that component as dirty
	Renderer::RenderScene& renderScene = GetModuleRegistry().GetModule<RendererModule>().GetRenderScene();
	auto view = ViewComponents<StaticMeshComponent, TransformComponent>();
	for (const EcsEntity& entity : view)
	{
		const StaticMeshComponent& meshComponent = view.GetComponents<StaticMeshComponent>(entity);
		if(!meshComponent.AssetHandle->IsLoaded())
		{
			continue;
		}
		else if(!renderScene.HasRenderProxy(entity))
		{
			// TODO: This should be moved to some streaming system
			CreateRenderProxy(entity);
			continue;
		}
		
		const TransformComponent& transformComponent = view.GetComponents<TransformComponent>(entity);
		
		
		renderScene.UpdateStaticMeshRenderProxy(entity, transformComponent.Transform);
	}
}

// TODO: move it to a separate system dedicated for camera updates
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
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	for (auto entity : Observer)
	{
		StaticMeshComponent& staticMeshComponent = Observer.GetComponents<StaticMeshComponent>(entity);
		if(!staticMeshComponent.AssetHandle->IsLoaded())
		{
			manager.LoadAssetAsync(staticMeshComponent.AssetHandle);
			continue;
		}
		
		// TODO: This should be moved to some streaming system
		CreateRenderProxy(entity);
	}
}

void RenderSystem::OnRemove(const OnRemoveObserverType::ObserverType& Observer)
{
	ZoneScopedN("RenderSystem::OnRemove");
	Renderer::RenderScene& renderScene = GetModuleRegistry().GetModule<RendererModule>().GetRenderScene();
	for (auto entity : Observer)
	{
		renderScene.DeleteRenderProxy(entity);
	}
}

void RenderSystem::CreateRenderProxy(EcsEntity Entity)
{
	Renderer::RenderScene& renderScene = GetModuleRegistry().GetModule<RendererModule>().GetRenderScene();
	const TransformComponent& transformComponent = GetComponent<TransformComponent>(Entity);
	const StaticMeshComponent& meshComponent = GetComponent<StaticMeshComponent>(Entity);
	
	renderScene.CreateStaticMeshRenderProxy(Entity, meshComponent.AssetHandle, transformComponent.Transform);
}
}
