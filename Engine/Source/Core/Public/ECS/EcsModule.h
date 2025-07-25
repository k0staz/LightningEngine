#pragma once


namespace LE
{
class EcsEntityManager;
class EcsComponentManager;
class EcsSystemManager;

class ECSModule
{
public:
	ECSModule() = default;

	void Initialize(EcsEntityManager* EntityManager, EcsComponentManager* ComponentManager, EcsSystemManager* SystemManager);

	EcsEntityManager* GetEntityManager() { return EntityManager; }
	EcsComponentManager* GetComponentManager() { return ComponentManager; }
	EcsSystemManager* GetSystemManager() { return SystemManager; }

private:
	EcsEntityManager* EntityManager;
	EcsComponentManager* ComponentManager;
	EcsSystemManager* SystemManager;
};
}
