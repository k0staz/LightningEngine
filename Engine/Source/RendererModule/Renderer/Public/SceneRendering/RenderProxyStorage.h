#pragma once
#include "Containers/SparseSet.h"
#include "RenderContributors/RenderContributerCore.h"

namespace LE::Renderer
{
struct RenderProxyState
{
	EcsEntity EntityId = EcsEntityNull;
	bool IsEnabled = true;

	RenderContributorTypeId MeshVariationTypeId = NullId{};
	RenderContributorId MeshVariationInstanceId = NullId{};

	RenderContributorTypeId MaterialVariationTypeId = NullId{};
	RenderContributorId MaterialVariationInstanceId = NullId{};
};

template<typename Entity>
class RenderProxyStorage : public SparseSet<Entity>
{
	using Traits = EcsTraits<Entity>;
public:
	using base_type = SparseSet<Entity>;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using SparseSet<Entity>::SparseSet;
	
	RenderProxyStorage()
		: base_type(base_type::Usage::Entity)
	{
	}
	
	RenderProxyState& AddProxy(Entity EntityId)
	{
		const Entity newEntity = *(base_type::Add(EntityId));
		return *GetCreateProxyState(EntityId);
	}
	
	RenderProxyState& GetProxyState(Entity EntityId)
	{
		LE_ASSERT_DESC(base_type::Has(EntityId), "Entity doesn't exist")
		return GetProxyStateRef(EntityId);
	}
	
protected:
	void Pop(const typename base_type::iterator Begin, const typename base_type::iterator End) override
	{
		for (typename base_type::iterator current = Begin; current != End; ++current)
		{
			RenderProxyState& state = GetProxyStateRef(base_type::GetSparseIndex(*current));
			state = {};
			std::swap(state, GetProxyStateRef(base_type::GetFreeListHead()));
			
			base_type::Pop(current, current + 1);
		}
	}

	void PopAll() override
	{
		for (RenderProxyState*& statePage : ProxyStates)
		{
			delete[] statePage;
		}
		ProxyStates.clear();
		
		base_type::PopAll();
	}
	
private:
	RenderProxyState& GetProxyStateRef(const size_type Position)
	{
		return ProxyStates[Position / Traits::PageSize][FastMod(Position, Traits::PageSize)];
	}
	
	RenderProxyState* GetCreateProxyState(const size_type Position)
	{
		const size_type pageIdx = Position / Traits::PageSize;
		if (pageIdx >= ProxyStates.size())
		{
			size_type current = ProxyStates.size();
			ProxyStates.resize(pageIdx + 1, nullptr);
			for (; current < ProxyStates.size(); ++current)
			{
				ProxyStates[current] = new RenderProxyState[Traits::PageSize];
			}
		}

		return ProxyStates[pageIdx] + FastMod(Position, Traits::PageSize);
	}
	
private:
	std::vector<RenderProxyState*> ProxyStates;
};
}
