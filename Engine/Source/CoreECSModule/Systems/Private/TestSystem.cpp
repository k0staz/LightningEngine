#include "TestSystem.h"

#include "CoreECSUpdatePasses.h"
#include "TestComponent.h"
#include "TransformComponent.h"
#include "ECS/Ecs.h"
#include "Math/MathUtils.h"
#include "Multithreading/UpdateJobs.h"
#include "Multithreading/UpdatePasses.h"
#include "tracy/Tracy.hpp"

namespace LE
{
void TestSystem::Initialize()
{
    TestSystemUpdate.GetDelegate().Attach<&TestSystem::Update>(this);
    TestSystemUpdate.WritesComponents<TransformComponent>();
    UpdatePass::AddJob<TestUpdatePass>(&TestSystemUpdate);
}

void TestSystem::Update(const float DeltaSeconds)
{
    ZoneScopedN("TestSystem::Update");
    static float time = 0.0f;

    time += DeltaSeconds;

    auto view = ViewComponents<TransformComponent, TestComponent>();
    for (auto entity : view)
    {
        TransformComponent& transformComponent = view.GetComponents<TransformComponent>(entity);

        Vector3F pos = transformComponent.Transform.GetPosition();

        static constexpr float freq = 0.2f;
        static constexpr float amp = 0.0002f;

        pos.Y += amp * Sin(PI * time * freq);
        pos.X += amp * Sin(PI * time * freq);

        transformComponent.Transform.SetPosition(pos);

        const float rotationSpeed = MathUtils::DegreesToRadians(10.0f);
        const float rotation = rotationSpeed * DeltaSeconds;
        transformComponent.Transform.RotateSelfY(rotation);
    }
}

void TestSystem::Shutdown()
{
}
}
