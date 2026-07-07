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
#include "Math/MathUtils.h"
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
    /*Path testModel = GetContentRoot() / "StaticMesh" / "GLTF" /"WallTileMaterial.glb";

    auto& importer = GetServiceRegistry().GetService<GLTFImporter>();
    importer.LoadAndConvertModel(testModel);*/

    Path testAsset = GetContentRoot() / "StaticMesh" / "Sphere.leasset";
    Path testMaterial = GetContentRoot() / "Materials" / "Rock064.leasset";
    Path wallAsset = GetContentRoot() / "StaticMesh" / "Cube.002.leasset";
    Path wallMaterial = GetContentRoot() / "Materials" / "Tiles143.leasset";

    AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
    AssetHandle<StaticMeshAsset> assetHandle = manager.GetAssetUsingPath<StaticMeshAsset>(testAsset);
    AssetHandle<MaterialInstanceAsset> materialHandle = manager.GetAssetUsingPath<MaterialInstanceAsset>(testMaterial);

    AssetHandle<StaticMeshAsset> wallAssetHandle = manager.GetAssetUsingPath<StaticMeshAsset>(wallAsset);
    AssetHandle<MaterialInstanceAsset> wallMaterialHandle = manager.GetAssetUsingPath<MaterialInstanceAsset>(wallMaterial);

    LE::EcsEntity rootEntity = Registry.CreateEntity();
    LE::EcsEntity meshRootEntity = Registry.CreateEntity(rootEntity);

    EcsEntity wallsEntity = Registry.CreateEntity(rootEntity);

    for (uint8 i = 0; i < 4; ++i)
    {
        EcsEntity wallEntity = Registry.CreateEntity(wallsEntity);
        auto& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(wallEntity);

        if (i == 0)
        {
            transformComponent.Transform.SetPosition(0.0f, 0.0f, 8.0f);
            transformComponent.Transform.RotateSelfX(MathUtils::DegreesToRadians(90.f));
        }
        else if (i == 1)
        {
            transformComponent.Transform.SetPosition(3.6f, -5.0f, 10.0f);
        }
        else if (i == 2)
        {
            transformComponent.Transform.SetPosition(-13.0f, 1.9f, 10.0f);
            transformComponent.Transform.RotateSelfZ(MathUtils::DegreesToRadians(90.f));
        }
        else if (i == 3)
        {
            transformComponent.Transform.SetPosition(13.0f, 1.9f, 10.0f);
            transformComponent.Transform.RotateSelfZ(MathUtils::DegreesToRadians(90.f));
        }

        auto& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(wallEntity);
        staticMeshComponent.AssetHandle = wallAssetHandle;

        auto& materialComponent = Registry.AddComponentToEntity<MaterialComponent>(wallEntity);
        materialComponent.AssetHandle = wallMaterialHandle;
    }

    {
        LE::EcsEntity entity = Registry.CreateEntity(meshRootEntity);
        LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
        transformComponent.Transform.SetPosition(0.0f, 0.0f, 2.0f);
        transformComponent.Transform.Scale(3.0f);

        LE::StaticMeshComponent& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
        staticMeshComponent.AssetHandle = assetHandle;

        MaterialComponent& materialComponent = Registry.AddComponentToEntity<MaterialComponent>(entity);
        materialComponent.AssetHandle = materialHandle;

        Registry.AddComponentToEntity<TestComponent>(entity);
    }

    {
        LE::EcsEntity entity = Registry.CreateEntity(meshRootEntity);
        auto& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
        transformComponent.Transform.SetPosition(3.0f, 0.0f, 0.0f);
        transformComponent.Transform.Scale(0.2f);

        auto& staticMeshComponent = Registry.AddComponentToEntity<LE::StaticMeshComponent>(entity);
        staticMeshComponent.AssetHandle = assetHandle;

        // TODO:: Rework this so this material is loaded from asset instance, and contributor is reused for the same assets
        auto& materialComponent = Registry.AddComponentToEntity<MaterialComponent>(entity);
        materialComponent.IsColor = true;
        materialComponent.Color = LinearColor::White();
    }

    {
        LE::EcsEntity entity = Registry.CreateEntity(rootEntity);
        LE::TransformComponent& transformComponent = Registry.AddComponentToEntity<LE::TransformComponent>(entity);
        transformComponent.Transform.SetPosition(0.0f, 0.0f, -6.0f);

        Registry.AddComponentToEntity<LE::CameraComponent>(entity);
    }
}
}
