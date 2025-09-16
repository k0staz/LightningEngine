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
	void Shutdown() override;

	REGISTER_UPDATE_JOB(RenderUpdateStaticMesh)
	void UpdateStaticMeshes(const float DeltaSeconds);

	REGISTER_UPDATE_JOB(RenderUpdateCamera)
	void UpdateCamera(const float DeltaSeconds);

	REGISTER_OBSERVER_JOB(OnAddObserver, ComponentChangeType::ComponentAdded, (StaticMeshComponent, TransformComponent), ())
	void OnAdd(const OnAddObserverType::ObserverType& Observer);

	REGISTER_OBSERVER_JOB(OnRemoveObserver, ComponentChangeType::ComponentAdded, (StaticMeshComponent, TransformComponent), ())
	void OnRemove(const OnRemoveObserverType::ObserverType& Observer);

};

REGISTER_ECS_SYSTEM(RenderSystem)
}
