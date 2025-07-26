#pragma once
#include "IWorld.h"
#include "ECS/EcsComponent.h"
#include "ECS/EcsSystem.h"
#include "SceneRendering/SceneView.h"

namespace LE
{
class World : public IWorld
{
public:
	void Init();
	void Shutdown();

	void Update();
	void PostUpdate();

	void SetPrimaryViewInfo(const Renderer::SceneViewInfo& ViewInfo) override { PrimaryViewInfo = ViewInfo; }
	const Renderer::SceneViewInfo& GetPrimaryViewInfo() const override { return PrimaryViewInfo; }

private:
	void InitTestData();

public:
	EcsEntityManager EntityManager;
	EcsComponentManager ComponentManager;
	EcsSystemManager SystemManager;

private:
	Renderer::SceneViewInfo PrimaryViewInfo;
};
}
