#include "ECS/EcsEntity.h"

#include "Log.h"

namespace LE
{
	using namespace std;
	
	EcsEntity::EcsEntity()
	{
		static atomic<uint64_t> nextId { ENTITY_ID_INVALID + 1 };
		EntityId = nextId.fetch_add(1);
	}

	EcsEntity::EcsEntity(EcsEntity&& Other) noexcept
	{
		EntityId = Other.EntityId;
	}

	EcsEntity& EcsEntity::operator=(EcsEntity&& Other) noexcept
	{
		EntityId = Other.EntityId;
		return *this;
	}

	bool EcsEntity::operator==(const EcsEntity& rhs) const noexcept
	{
		return this->EntityId == rhs.EntityId;
	}
	
	EcsEntityManager::EcsEntityManager()
	{
		EntityContainer.reserve(ENTITY_NUMBER);
	}
	
	EcsEntity* EcsEntityManager::GetEntityById(EntityId EntityId) const
	{
		auto entity = EntityLookUpTable.find(EntityId);
		if (entity != EntityLookUpTable.end())
		{
			return entity->second;
		}

		return nullptr;
	}

	EcsEntity* EcsEntityManager::CreateEntity()
	{
		EcsEntity& newEntity = EntityContainer.emplace_back();
		EntityLookUpTable[newEntity.GetId()] = &newEntity;

		return &newEntity;
	}

	bool EcsEntityManager::DeleteEntityById(EntityId EntityId)
	{
		EcsEntity* entity = GetEntityById(EntityId);
		if (!entity)
		{
			return false;
		}

		auto entityIt = find(EntityContainer.begin(), EntityContainer.end(), *entity);
		EntityContainer.erase(entityIt);

		LE_INFO("Deleted entity with ID: {}", EntityId);
		return true;
	}
}
