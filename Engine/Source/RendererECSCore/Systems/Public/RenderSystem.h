#pragma once
#include "MaterialComponent.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"
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

	REGISTER_OBSERVER_JOB(OnAddObserver, ComponentChangeType::ComponentAdded, (StaticMeshComponent, TransformComponent, MaterialComponent), ())
	void OnAdd(const OnAddObserverType::ObserverType& Observer);

	REGISTER_OBSERVER_JOB(OnRemoveObserver, ComponentChangeType::ComponentRemoved, (StaticMeshComponent, TransformComponent, MaterialComponent), ())
	void OnRemove(const OnRemoveObserverType::ObserverType& Observer);

private:
	void CreateRenderProxy(EcsEntity Entity);

};

REGISTER_ECS_SYSTEM(RenderSystem)
}
