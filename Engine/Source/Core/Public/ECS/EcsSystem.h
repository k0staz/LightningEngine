#pragma once

#include <vector>
#include <memory>

#include "Templates/NonCopyable.h"

namespace LE
{
// TODO: Temp decision until I came up with something better
#define REGISTER_ECS_SYSTEM(SystemName)

	class EcsSystem : public NonCopyable
	{
	public:
		EcsSystem() {};
		virtual ~EcsSystem() = default;

		virtual void Initialize() = 0;
		virtual void Update(const float DeltaSeconds) = 0;
		virtual void Shutdown() = 0;
	};

	class EcsSystemManager
	{
	public:
		EcsSystemManager() = default;
		EcsSystemManager(EcsSystemManager&) = delete;
		EcsSystemManager(EcsSystemManager&&) = delete;

		void Shutdown();

		template<typename System>
		void RegisterSystem();

	private:
		std::vector<std::unique_ptr<EcsSystem>> Systems;
	};

	template<typename System>
	inline void EcsSystemManager::RegisterSystem()
	{
		Systems.emplace_back(std::make_unique<System>());
		Systems.back()->Initialize();
	}

}
