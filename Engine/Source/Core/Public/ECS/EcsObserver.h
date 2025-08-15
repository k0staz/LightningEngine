#pragma once
#include "EcsDefinitions.h"
#include "EcsRegistry.h"
#include "EcsSignals.h"
#include "EcsStorageView.h"

namespace LE
{
template <typename, typename>
class EcsObserver;

template <typename... Components, typename... ExcludedComponents>
class EcsObserver<IncludedComponentTypes<Components...>, ExcludedComponentTypes<ExcludedComponents...>>
	: public EcsStorageView<IncludedComponentTypes<Components...>, ExcludedComponentTypes<ExcludedComponents...>>
{
	using base_type = EcsStorageView<IncludedComponentTypes<Components...>, ExcludedComponentTypes<ExcludedComponents...>>;

public:
	using common_type = typename base_type::common_type;
	using entity_type = typename base_type::entity_type;
	using size_type = typename base_type::size_type;
	using difference_type = typename base_type::difference_type;
	using iterator = typename base_type::iterator;

	EcsObserver() noexcept
		: base_type{},
		  ObserverType{},
		  ObservedEntities(SparseSet<entity_type>::Usage::Component)
		  , Registry{}
	{
	}

	EcsObserver(ComponentChangeType InObserverType, EcsRegistry<entity_type>& InRegistry, base_type View) noexcept
		: base_type(View)
		  , ObserverType(InObserverType)
		  , ObservedEntities(SparseSet<entity_type>::Usage::Component)
		  , Registry(&InRegistry)
	{
		SubscribeToComponentChanges();
	}

	EcsObserver(const EcsObserver&) = delete;

	EcsObserver(EcsObserver&& Other) noexcept
	{
		Swap(Other);
		SubscribeToComponentChanges();
	}

	EcsObserver& operator=(const EcsObserver&) = delete;

	EcsObserver& operator=(EcsObserver&& Other) noexcept
	{
		UnsubscribeFromComponentChanges();
		Swap(Other);
		SubscribeToComponentChanges();
		return *this;
	}

	~EcsObserver() override
	{
		UnsubscribeFromComponentChanges();
	}

	void Swap(EcsObserver& Other)
	{
		std::swap(ObserverType, Other.ObserverType);
		std::swap(ObservedEntities, Other.ObservedEntities);
		std::swap(Registry, Other.Registry);

		base_type::Swap(Other);
	}

	const common_type* GetLeadingStorage() const noexcept override
	{
		return &ObservedEntities;
	}

	void OnStorageChange(const entity_type Entity)
	{
		if (ObservedEntities.Has(Entity))
		{
			return;
		}
		ObservedEntities.Add(Entity);
	}

	void OnFromStorageRemoved(const entity_type Entity)
	{
		if (ObservedEntities.Has(Entity))
		{
			ObservedEntities.Delete(Entity);
		}
	}

	void ClearObserverEntities()
	{
		ObservedEntities.Clear();
	}

private:
	void SubscribeToComponentChanges() noexcept
	{
		EcsRegistry<entity_type>& registry = GetRegistry();
		switch (ObserverType)
		{
		case ComponentChangeType::ComponentAdded:
			((ComponentsSinks.emplace_back(registry.template GetOnAddedSink<typename Components::value_type>())), ...);
			break;
		case ComponentChangeType::ComponentRemoved:
			((ComponentsSinks.emplace_back(registry.template GetOnRemovedSink<typename Components::value_type>())), ...);
			break;
		case ComponentChangeType::ComponentUpdated:
			((ComponentsSinks.emplace_back(registry.template GetOnUpdatedSink<typename Components::value_type>())), ...);
			break;
		}

		for (auto& sink : ComponentsSinks)
		{
			sink.template Attach<&EcsObserver::OnStorageChange>(this);
		}

		if (ObserverType != ComponentChangeType::ComponentRemoved)
		{
			((RemovedSinks.emplace_back(registry.template GetOnRemovedSink<typename Components::value_type>())), ...);
			for (auto& sink : RemovedSinks)
			{
				sink.template Attach<&EcsObserver::OnFromStorageRemoved>(this);
			}
		}
	}

	void UnsubscribeFromComponentChanges() noexcept
	{
		for (auto& sink : ComponentsSinks)
		{
			sink.template Detach<&EcsObserver::OnStorageChange>(this);
		}
		ComponentsSinks.clear();

		if (ObserverType != ComponentChangeType::ComponentRemoved)
		{
			for (auto& sink : RemovedSinks)
			{
				sink.template Detach<&EcsObserver::OnFromStorageRemoved>(this);
			}
			RemovedSinks.clear();
		}
	}

	EcsRegistry<entity_type>& GetRegistry() noexcept
	{
		LE_ASSERT_DESC(Registry != nullptr, "Invalid ECS observer")
		return *Registry;
	}

private:
	ComponentChangeType ObserverType;
	SparseSet<entity_type> ObservedEntities;
	EcsRegistry<entity_type>* Registry = nullptr;
	std::vector<Sink<Signal<void(const entity_type)>>> ComponentsSinks;
	std::vector<Sink<Signal<void(const entity_type)>>> RemovedSinks;
};
}
