#pragma once

#include "CoreECSUpdatePasses.h"
#include "ECS/Ecs.h"
#include "ECS/EcsObserver.h"
#include "ECS/EcsSystem.h"
#include "Components/TransformComponent.h"
#include "Multithreading/UpdateJobs.h"

namespace LE
{
class TestSystem : public EcsSystem
{
public:
	void Initialize() override;
	void Update(const float DeltaSeconds);
	REGISTER_UPDATE_JOB(TestSystemUpdate)
	void Shutdown() override;

};

REGISTER_ECS_SYSTEM(TestSystem)
}
