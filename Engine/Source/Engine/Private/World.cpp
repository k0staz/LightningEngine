#include "World.h"

#include "GLTFImporter.h"
#include "CameraComponent.h"
#include "MaterialComponent.h"
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
	Registry.Shutdown();
}

void World::InitTestData()
{
	/*Path testModel = GetContentRoot() / "StaticMesh" / "GLTF" /"SphereWithRockMaterial.glb";

	auto& importer = GetServiceRegistry().GetService<GLTFImporter>();
	importer.LoadAndConvertModel(testModel);*/
	
	Path testAsset = GetContentRoot() / "StaticMesh" / "Sphere.leasset";
	Path testMaterial = GetContentRoot() / "Materials" / "Rock064.leasset";

	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	AssetHandle<StaticMeshAsset> assetHandle = manager.GetAssetUsingPath<StaticMeshAsset>(testAsset);
	AssetHandle<MaterialInstanceAsset> materialHandle = manager.GetAssetUsingPath<MaterialInstanceAsset>(testMaterial);

	LE::EcsEntity rootEntity = Registry.CreateEntity();
	LE::EcsEntity meshRootEntity = Registry.CreateEntity(rootEntity);

	{
		LE::EcsEntity entity = Registry.CreateEntity(meshRootEntity);
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(0.0f, 0.0f, 5.0f);
		transformComponent.Transform.RotateSelfX(1.2f);

		LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
		staticMeshComponent.AssetHandle = assetHandle;

		MaterialComponent& materialComponent = Registry.AddComponentToEntity<MaterialComponent>(entity);
		materialComponent.AssetHandle = materialHandle;

		Registry.AddComponentToEntity<TestComponent>(entity);
	}
	
	{
		LE::EcsEntity entity = Registry.CreateEntity(rootEntity);
		LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
		transformComponent.Transform.SetPosition(0.0f, 0.0f, -10.0f);

		Registry.AddComponentToEntity<LE::CameraComponent>(entity);
	}
}
}
