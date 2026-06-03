#pragma once
#include "IWorld.h"
#include "ECS/EcsComponent.h"
#include "ECS/EcsRegistry.h"
#include "ECS/EcsSystem.h"
#include "SceneRendering/SceneView.h"
#include "Service/ServiceRegistry.h"

namespace LE
{
class World : public IWorld
{
public:
	void Init();
	void Shutdown();

	void SetPrimaryViewInfo(const Renderer::SceneViewInfo& ViewInfo) override { PrimaryViewInfo = ViewInfo; }
	const Renderer::SceneViewInfo& GetPrimaryViewInfo() const override { return PrimaryViewInfo; }

	void InitTestData();
	

public:
	EcsRegistry<EcsEntity> Registry;
	EcsSystemRegistry SystemRegistry;

private:
	Renderer::SceneViewInfo PrimaryViewInfo;
};
}
