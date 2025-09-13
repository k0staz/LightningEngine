#pragma once
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "ECS/Ecs.h"
#include "ECS/EcsObserver.h"
#include "ECS/EcsSystem.h"
#include "Multithreading/UpdateJobs.h"

namespace LE
{
class RenderSystem : public EcsSystem
{
public:
	void Initialize() override;
	void Update(const float DeltaSeconds) override;
	REGISTER_UPDATE_JOB(RenderUpdate)
	void Shutdown() override;

private:
	EcsObserver<ObservedComponentTypes<StaticMeshComponent, TransformComponent>, FilteredComponentTypes<>> OnAddObserver;
	EcsObserver<ObservedComponentTypes<StaticMeshComponent, TransformComponent>, FilteredComponentTypes<>> OnRemoveObserver;
};

REGISTER_ECS_SYSTEM(RenderSystem)
}
