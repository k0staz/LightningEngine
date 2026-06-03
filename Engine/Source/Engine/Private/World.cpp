#include "World.h"

#include "FBXImporter.h"
#include "StaticMeshAsset.h"
#include "CameraComponent.h"
#include "StaticMeshComponent.h"
#include "TestComponent.h"
#include "TransformComponent.h"
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
void World::Init()
{
	{
		UniquePtr<ECSModule> module = std::make_unique<ECSModule>();
		module->Initialize(&Registry, &SystemRegistry);
		RegisterECSModule(std::move(module));
	};
}

void World::Shutdown()
{
	SystemRegistry.Shutdown();
}

void World::InitTestData()
{
	/*Path testModel = GetContentRoot() / "StaticMesh" / "FBX" /"Sphere.fbx";

	FBXImporter& fbxImporter = GetServiceRegistry().GetService<FBXImporter>();
	fbxImporter.LoadAndConvertFbxModelAsync(testModel);*/
	
	Path testAsset = GetContentRoot() / "StaticMesh" / "Sphere.leasset";
	
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	AssetHandle<StaticMeshAsset> assetHandle = manager.GetAssetUsingPath<StaticMeshAsset>(testAsset);

	LE::EcsEntity rootEntity = Registry.CreateEntity();
	LE::EcsEntity meshRootEntity = Registry.CreateEntity(rootEntity);
	
	{
		LE::EcsEntity entity = Registry.CreateEntity(meshRootEntity);
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(0.0f, 2.0f, 5.0f);
		transformComponent.Transform.RotateSelfZ(1.2f);
		transformComponent.Transform.RotateSelfX(1.2f);

		LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
		staticMeshComponent.AssetHandle = assetHandle;
		
		Registry.AddComponentToEntity<TestComponent>(entity);
	}

	{
		LE::EcsEntity entity = Registry.CreateEntity(meshRootEntity);
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(-5.0f, -2.0f, 5.0f);
		transformComponent.Transform.RotateSelfX(1.2f);

		LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
		staticMeshComponent.AssetHandle = assetHandle;
		
		Registry.AddComponentToEntity<TestComponent>(entity);
	}

	{
		LE::EcsEntity entity = Registry.CreateEntity(meshRootEntity);
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(5.0f, -2.0f, 5.0f);
		transformComponent.Transform.RotateSelfX(1.2f);

		LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
		staticMeshComponent.AssetHandle = assetHandle;
		
		Registry.AddComponentToEntity<TestComponent>(entity);
	}
	
	{
		LE::EcsEntity entity = Registry.CreateEntity(rootEntity);
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(0.0f, 0.0f, 0.0f);

		Registry.AddComponentToEntity<LE::CameraComponent>(entity);
	}
}
}
