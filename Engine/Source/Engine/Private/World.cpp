#include "World.h"

#include "../../CoreECS/Generated/Public/ECSSystemAutoRegistration.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Containers/Array.h"
#include "ECS/Ecs.h"
#include "ECS/EcsModule.h"
#include "EventCore/EventManager.h"
#include "StaticMesh/StaticMeshRendering.h"
#include "Time/Clock.h"

namespace LE
{
static float locCalculateDeltaSeconds()
{
	static Clock::TimePoint prev = Clock::Now();
	const Clock::TimePoint cur = Clock::Now();
	const float deltaSeconds = Clock::GetSecondsBetween(prev, cur);
	prev = cur;

	return deltaSeconds;
}

void World::Init()
{
	{
		UniquePtr<ECSModule> module = std::make_unique<ECSModule>();
		module->Initialize(&Registry, &SystemManager);
		RegisterECSModule(std::move(module));

		LE::ECSSystemRegistration::RegisterAllSystems(SystemManager); // TODO: Temp decision until I came up with something better
	}

	InitTestData();
}

void World::Shutdown()
{
	SystemManager.Shutdown();
}

void World::Update()
{
	const float deltaSeconds = locCalculateDeltaSeconds();
	SystemManager.Update(deltaSeconds);
}

void World::PostUpdate()
{
}

void World::InitTestData()
{
	Array<Renderer::StaticMeshVertex> vertices = {
		// Front (+Z)
		{{-1, -1, 1, 1}, {0, 0, 1}, {0, 1}}, // 0
		{{-1, 1, 1, 1}, {0, 0, 1}, {1, 1}}, // 1
		{{1, 1, 1, 1}, {0, 0, 1}, {1, 0}}, // 2
		{{1,-1, 1, 1}, {0, 0, 1}, {0, 0}}, // 3
		{{-1,1, -1, 1}, {0, 0, 1}, {0, 0}}, // 4
		{{1,1, -1, 1}, {0, 0, 1}, {0, 0}}, // 5
		{{1,-1, -1, 1}, {0, 0, 1}, {0, 0}}, // 6
		{{-1,-1, -1, 1}, {0, 0, 1}, {0, 0}}, // 7
	};

	Array<uint16> indices = {
			0, 1 ,2, 2, 3, 0, // Front
			1, 4 ,5, 5, 2, 1, // Top
			3, 2 ,5, 5, 6, 3, // Right
			7, 4,1, 1, 0, 7, // Left
			5, 4,7, 7, 6, 5, // back
			7, 0,3, 3, 6, 7, // bottom
	};

	{
		LE::EcsEntity entity = Registry.CreateEntity();
		LE::TransformComponent transformComponent;
		transformComponent.Transform.SetPosition(0.0f, 2.0f, 5.0f);
		transformComponent.Transform.RotateSelfZ(1.2f);
		transformComponent.Transform.RotateSelfX(1.2f);
		Registry.AddComponentToEntity<TransformComponent>(entity, transformComponent);

		LE::StaticMeshComponent staticMeshComponent;
		staticMeshComponent.RenderData = new Renderer::StaticMeshRenderData();

		staticMeshComponent.RenderData->PrimitiveType = RHI::PrimitiveType::TriangleList;

		staticMeshComponent.RenderData->VertexBuffers.Init(vertices);
		staticMeshComponent.RenderData->IndexBuffer.Init(indices);

		staticMeshComponent.RenderData->InitResources();

		staticMeshComponent.MeshMaterial = Renderer::Material::GetMaterialByName("BaseMaterial");
		Registry.AddComponentToEntity<StaticMeshComponent>(entity, staticMeshComponent);
	}

	{
		LE::EcsEntity entity = Registry.CreateEntity();
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(0.0f, -2.0f, 5.0f);
		transformComponent.Transform.RotateSelfX(1.2f);

		LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
		staticMeshComponent.RenderData = new Renderer::StaticMeshRenderData();

		staticMeshComponent.RenderData->PrimitiveType = RHI::PrimitiveType::TriangleList;

		staticMeshComponent.RenderData->VertexBuffers.Init(vertices);
		staticMeshComponent.RenderData->IndexBuffer.Init(indices);

		staticMeshComponent.RenderData->InitResources();

		staticMeshComponent.MeshMaterial = Renderer::Material::GetMaterialByName("BaseMaterial");
	}

	{
		LE::EcsEntity entity = Registry.CreateEntity();
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(0.0f, 0.0f, 0.0f);

		Registry.AddComponentToEntity<LE::CameraComponent>(entity);
	}
}
}
