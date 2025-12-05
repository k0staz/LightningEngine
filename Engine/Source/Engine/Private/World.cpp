#include "World.h"

#include "../../AutoRegistration/Generated/Public/ECSSystemAutoRegistration.h"
#include "Assets/StaticMeshAsset.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Containers/Array.h"
#include "ECS/Ecs.h"
#include "ECS/EcsModule.h"
#include "EventCore/EventManager.h"
#include "FileManager/FileManager.h"
#include "Misc/Paths.h"
#include "Archive/Archive.h"
#include "AssetManager/AssetManager.h"
#include "Time/Clock.h"

namespace LE
{
void SaveTestStaticMesh()
{
	AssetRegistry& reg = GetServiceRegistry().GetService<AssetRegistry>();
	
	Path savePathTest = GetContentRoot() / "StaticMesh" / "TestAsset.leasset";

	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	AssetHandle<TestAsset> assetHandle = manager.GetAssetUsingPath<TestAsset>(savePathTest);
	manager.LoadAssetAsync(assetHandle);
}

void World::Init()
{
	{
		UniquePtr<ECSModule> module = std::make_unique<ECSModule>();
		module->Initialize(&Registry, &SystemManager);
		RegisterECSModule(std::move(module));
		AutoRegistration::RegisterAllSystems(SystemManager); // TODO: These needs to be moved out of there, and we need to create an registry for systems
	}

	InitTestData();
}

void World::Shutdown()
{
	SystemManager.Shutdown();
}

void World::InitTestData()
{
	Path testAsset = GetContentRoot() / "StaticMesh" / "NewStaticMesh.leasset";

	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	AssetHandle<StaticMeshAsset> assetHandle = manager.GetAssetUsingPath<StaticMeshAsset>(testAsset);
	
	{
		LE::EcsEntity entity = Registry.CreateEntity();
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(0.0f, 2.0f, 5.0f);
		transformComponent.Transform.RotateSelfZ(1.2f);
		transformComponent.Transform.RotateSelfX(1.2f);

		LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
		staticMeshComponent.AssetHandle = assetHandle;
	}

	{
		LE::EcsEntity entity = Registry.CreateEntity();
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(-5.0f, -2.0f, 5.0f);
		transformComponent.Transform.RotateSelfX(1.2f);

		LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
		staticMeshComponent.AssetHandle = assetHandle;
	}

	{
		LE::EcsEntity entity = Registry.CreateEntity();
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(5.0f, -2.0f, 5.0f);
		transformComponent.Transform.RotateSelfX(1.2f);

		LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
		staticMeshComponent.AssetHandle = assetHandle;
	}

	{
		LE::EcsEntity entity = Registry.CreateEntity();
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(0.0f, 0.0f, 0.0f);

		Registry.AddComponentToEntity<LE::CameraComponent>(entity);
	}
}
}
