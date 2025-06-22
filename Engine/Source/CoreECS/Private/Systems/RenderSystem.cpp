#include "Systems/RenderSystem.h"

#include "EngineGlobals.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Ecs.h"

namespace LE
{
void RenderSystem::Initialize()
{
	ComponentMask archetype;
	archetype.set(GetComponentTypeId<StaticMeshComponent>());
	archetype.set(GetComponentTypeId<TransformComponent>());

	ArchetypeMatchListener = [this](const ArchetypeMatched& Event) { OnArchetypeMatched(Event); };
	ArchetypeUnmatchListener = [this](const ArchetypeUnmatched& Event) { OnArchetypeUnmatched(Event); };

	gEventManager.ListenToArchetypeMatchedEvent(archetype, ArchetypeMatchListener);
	gEventManager.ListenToArchetypeUnmatchedEvent(archetype, ArchetypeUnmatchListener);
}

void RenderSystem::Update(const float DeltaSeconds)
{
	Renderer::RenderScene& renderScene = GetRendererModule()->GetRenderScene();
	const auto& proxyMap = renderScene.GetProxyMap();

	ComponentMask archetype;
	archetype.set(GetComponentTypeId<StaticMeshComponent>());
	archetype.set(GetComponentTypeId<TransformComponent>());

	auto entities = GetArchetypeMatchedEntities(archetype);
	for (const EntityId& it : entities)
	{
		const TransformComponent* transformComponent = ReadComponent<TransformComponent>(it);
		if (!transformComponent)
		{
			LE_ASSERT(false)
			continue;
		}

		if (!proxyMap.contains(it))
		{
			continue;
		}

		Renderer::RenderObjectProxy* proxy = proxyMap.at(it);
		proxy->SetTransform(transformComponent->Transform);
		proxy->UpdateConstantBuffer(Renderer::RenderCommandList::Get());
	}

	ComponentMask cameraArchetype;
	cameraArchetype.set(GetComponentTypeId<CameraComponent>());
	cameraArchetype.set(GetComponentTypeId<TransformComponent>());
	auto cameraEntities = GetArchetypeMatchedEntities(cameraArchetype);
	for (const EntityId& it : cameraEntities)
	{
		// TODO: For now we always assume that we have one camera, later we will need to handle multiple active cameras out of all cameras
		const TransformComponent* transformComponent = ReadComponent<TransformComponent>(it);
		if (!transformComponent)
		{
			LE_ASSERT(false)
			continue;
		}

		const CameraComponent* cameraComponent = ReadComponent<CameraComponent>(it);
		if (!cameraComponent)
		{
			LE_ASSERT(false)
			continue;
		}

		Renderer::SceneViewInfo newViewInfo;
		newViewInfo.FOV = cameraComponent->FOV;
		newViewInfo.ViewTransform = transformComponent->Transform;
		GetWorld()->SetPrimaryViewInfo(newViewInfo);
	}
}

void RenderSystem::Shutdown()
{
	gEventManager.Unsubscribe(ArchetypeMatched::GetStaticEventType(), ArchetypeMatchListener.target_type().name());
	gEventManager.Unsubscribe(ArchetypeUnmatched::GetStaticEventType(), ArchetypeUnmatchListener.target_type().name());
}

void RenderSystem::OnArchetypeMatched(const ArchetypeMatched& Event)
{
	Renderer::RenderScene& renderScene = GetRendererModule()->GetRenderScene();

	const StaticMeshComponent* staticMeshComponent = ReadComponent<StaticMeshComponent>(Event.EntityId);
	if (!staticMeshComponent)
	{
		LE_ASSERT(false)
		return;
	}

	const TransformComponent* transformComponent = ReadComponent<TransformComponent>(Event.EntityId);
	if (!transformComponent)
	{
		LE_ASSERT(false)
		return;
	}

	Renderer::StaticMeshRenderProxy* proxy = renderScene.CreateStaticMeshRenderProxy(
		Event.EntityId, staticMeshComponent->RenderData, staticMeshComponent->MeshMaterial);
	if (!proxy)
	{
		LE_ASSERT(false)
		return;
	}

	proxy->SetTransform(transformComponent->Transform);
	proxy->CreateConstantBuffer();
}

void RenderSystem::OnArchetypeUnmatched(const ArchetypeUnmatched& Event)
{
	Renderer::RenderScene& renderScene = GetRendererModule()->GetRenderScene();
	renderScene.DeleteRenderObjectProxy(Event.EntityId);
}
}
