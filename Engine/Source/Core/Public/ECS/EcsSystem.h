#pragma once

#include <vector>
#include <memory>

namespace LE
{
	class EcsSystem
	{
	public:
		EcsSystem() {};
		virtual ~EcsSystem() = default;

		virtual void Initialize() = 0;
		virtual void Update() = 0;
		virtual void Shutdown() = 0;
	};

	class EcsSystemManager
	{
	public:
		EcsSystemManager();
		EcsSystemManager(EcsSystemManager&) = delete;
		EcsSystemManager(EcsSystemManager&&) = delete;

		void Update();

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