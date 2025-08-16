#include "Systems/TestSystem.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "ECS/Ecs.h"

namespace LE
{
	void TestSystem::Initialize()
	{
	}

	void TestSystem::Update(const float DeltaSeconds)
	{
		static float time = 0.0f;

		time += DeltaSeconds;

		auto view = ViewComponents<TransformComponent>(ExcludedComponentTypes<CameraComponent>());
		for (auto entity : view)
		{
			TransformComponent& transformComponent = view.GetComponents<TransformComponent>(entity);

			Vector3F pos = transformComponent.Transform.GetPosition();

			static constexpr float freq = 0.00002f;
			static constexpr float amp = 2.2f;

			pos.Y += amp * Sin(TWO_PI * time * freq);

			transformComponent.Transform.SetPosition(pos);

			LE_INFO("Change transform to X {} Y {} Z {} for Entity {}", pos.X, pos.Y, pos.Z, entity);
		}
	}

	void TestSystem::Shutdown()
	{
		
	}
}