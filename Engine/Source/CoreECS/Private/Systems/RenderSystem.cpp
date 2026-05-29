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
	OnAddObserver.ReadsComponents<TransformComponent>();
	OnAddObserver.WritesComponents<StaticMeshComponent>();
	OnAddObserver.AddsResources<Renderer::StaticMeshRenderProxy>();
	OnAddObserver.GetDelegate().Attach<&RenderSystem::OnAdd>(this);
	UpdatePass::AddJob<RenderPass>(&OnAddObserver);

	OnRemoveObserver.DeletesResources<Renderer::StaticMeshRenderProxy>();
	OnRemoveObserver.GetDelegate().Attach<&RenderSystem::OnRemove>(this);
	UpdatePass::AddJob<RenderPass>(&OnRemoveObserver);

	RenderUpdateStaticMesh.GetDelegate().Attach<&RenderSystem::UpdateStaticMeshes>(this);
	RenderUpdateStaticMesh.ReadsComponents<TransformComponent>();
	RenderUpdateStaticMesh.WritesComponents<StaticMeshComponent>();
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
		const StaticMeshComponent& meshComponent = view.GetComponents<StaticMeshComponent>(entity);
		if(!meshComponent.AssetHandle->IsLoaded())
		{
			continue;
		}
		else if(!renderScene.HasStaticMeshRenderProxy(entity))
		{
			CreateRenderProxy(entity);
			continue;
		}
		
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

	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	for (auto entity : Observer)
	{
		StaticMeshComponent& staticMeshComponent = Observer.GetComponents<StaticMeshComponent>(entity);
		if(!staticMeshComponent.AssetHandle->IsLoaded())
		{
			manager.LoadAssetAsync(staticMeshComponent.AssetHandle);
			continue;
		}
		
		CreateRenderProxy(entity);
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

void RenderSystem::CreateRenderProxy(EcsEntity Entity)
{
	Renderer::RenderScene& renderScene = GetRendererModule()->GetRenderScene();
	const TransformComponent& transformComponent = GetComponent<TransformComponent>(Entity);
	StaticMeshComponent& meshComponent = GetComponent<StaticMeshComponent>(Entity);

	// TODO: Do we really need to store Render Data and Mesh Material in the component? Perhaps it could be stored in proxy
	meshComponent.RenderData = new Renderer::StaticMeshRenderData();
	const StaticMeshAsset& asset = meshComponent.AssetHandle.GetAssetRef();
	meshComponent.RenderData->PrimitiveType = asset.PrimitiveType;
	meshComponent.RenderData->VertexBuffers.Init(asset.Vertices);
	meshComponent.RenderData->IndexBuffer.Init(asset.Indices);
	meshComponent.RenderData->InitResources();

	meshComponent.MeshMaterial = Renderer::Material::GetMaterialByName("BaseMaterial");
	
	renderScene.CreateStaticMeshRenderProxy(Entity, transformComponent.Transform, meshComponent.RenderData, meshComponent.MeshMaterial);
}
}
