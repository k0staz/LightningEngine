#include "Systems/TestSystem.h"

#include "CoreECSUpdatePasses.h"
#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "ECS/Ecs.h"
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

		auto view = ViewComponents<TransformComponent>(ExcludedComponentTypes<CameraComponent>());
		for (auto entity : view)
		{
			TransformComponent& transformComponent = view.GetComponents<TransformComponent>(entity);

			Vector3F pos = transformComponent.Transform.GetPosition();

			static constexpr float freq = 0.2f;
			static constexpr float amp = 0.0002f;

			pos.Y += amp * Sin(PI * time * freq);
			pos.X += amp * Sin(PI * time * freq);

			transformComponent.Transform.SetPosition(pos);
		}
	}

	void TestSystem::Shutdown()
	{
		
	}
}
