#pragma once

#include <vector>
#include <memory>

#include "Templates/NonCopyable.h"

namespace LE
{
// TODO: Temp decision until I came up with something better
#define REGISTER_ECS_SYSTEM(SystemName)

class EcsSystem : public NonCopyable {
public:
	EcsSystem() {};
	virtual ~EcsSystem() = default;

	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
};

/**
 * @brief Ecs System registry which holds all systems used in the given world.
 * Systems should be registered at the start-up, and registering a system during runtime is undefined.
 */
class EcsSystemRegistry {
public:
	EcsSystemRegistry() = default;
	EcsSystemRegistry(EcsSystemRegistry&) = delete;
	EcsSystemRegistry(EcsSystemRegistry&&) = delete;

	void Shutdown();

	/**
	 * @brief Registers a system, which will be used for a world, which owns this registry.
	 * Not thread safe.
	 * @tparam System System Type which should be registered
	 */
	template <typename System>
	void RegisterSystem();

private:
	std::vector<std::unique_ptr<EcsSystem>> Systems;
};

template <typename System>
inline void EcsSystemRegistry::RegisterSystem()
{
	Systems.emplace_back(std::make_unique<System>());
	Systems.back()->Initialize();
}

}
