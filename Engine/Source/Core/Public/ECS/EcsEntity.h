#pragma once

#include <cinttypes>
#include <vector>
#include <unordered_map>

namespace LE
{
	inline constexpr std::uint64_t ENTITY_ID_INVALID = 0;
	inline constexpr size_t ENTITY_NUMBER = 100;


	using EntityId = std::uint64_t;

	class EcsEntity
	{
	public:
		EcsEntity();
		EcsEntity(const EcsEntity&) = delete;
		EcsEntity(EcsEntity&& Other) noexcept;
		EcsEntity& operator=(const EcsEntity&) = delete;
		EcsEntity& operator=(EcsEntity&& Other) noexcept;

		inline EntityId GetId() const { return EntityId; }

		bool operator==(const EcsEntity& rhs) const noexcept;
	private:
		EntityId EntityId;
	};

	class EcsEntityManager
	{
	public:
		EcsEntityManager();
		EcsEntityManager(EcsEntityManager&) = delete;
		EcsEntityManager(EcsEntityManager&&) = delete;

		EcsEntity* GetEntityById(EntityId EntityId) const;
		EcsEntity* CreateEntity();
		bool DeleteEntityById(EntityId EntityId);

	private:
		std::vector<EcsEntity> EntityContainer;
		std::unordered_map<EntityId, EcsEntity*> EntityLookUpTable;
	};
}
