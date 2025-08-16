#pragma once

#include "ECS/Ecs.h"
#include "ECS/EcsObserver.h"
#include "ECS/EcsSystem.h"
#include "Components/TransformComponent.h"

namespace LE
{

	class TestSystem : public EcsSystem
	{
	public:
		void Initialize() override;
		void Update(const float DeltaSeconds) override;
		void Shutdown() override;

	};

	REGISTER_ECS_SYSTEM(TestSystem)
}