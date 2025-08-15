#pragma once
#include "SparseSet.h"
#include "ECS/EcsComponent.h"
#include "ECS/EcsSignals.h"

namespace LE
{
template <typename ContainerType, uint64 PageSize>
class EcsStorageIterator
{
	using value_ptr = typename ContainerType::value_type;
	using element = typename std::pointer_traits<value_ptr>::element_type;

	using raw_ptr = std::conditional_t<
		std::is_const_v<ContainerType>,
		const element*,
		element*>;

	using iterator_traits = std::iterator_traits<raw_ptr>;

public:
	using value_type = typename iterator_traits::value_type;
	using pointer = typename iterator_traits::const_pointer;
	using reference = typename iterator_traits::const_reference;
	using difference_type = typename iterator_traits::difference_type;
	using iterator_category = std::random_access_iterator_tag;

	constexpr EcsStorageIterator() noexcept = default;

	constexpr EcsStorageIterator(ContainerType* ContainerPtr, const difference_type InOffset) noexcept
		: Container(ContainerPtr)
		  , Offset(InOffset)
	{
	}

	constexpr EcsStorageIterator& operator++() noexcept
	{
		--Offset;
		return *this;
	}

	constexpr EcsStorageIterator operator++(int) noexcept
	{
		EcsStorageIterator current = *this;
		operator++();
		return current;
	}

	constexpr EcsStorageIterator& operator--() noexcept
	{
		++Offset;
		return *this;
	}

	constexpr EcsStorageIterator operator--(int) noexcept
	{
		EcsStorageIterator current = *this;
		operator--();
		return current;
	}

	constexpr EcsStorageIterator& operator+=(const difference_type Value) noexcept
	{
		Offset -= Value;
		return *this;
	}

	constexpr EcsStorageIterator operator+(const difference_type Value) const noexcept
	{
		EcsStorageIterator copy = *this;
		return copy += Value;
	}

	constexpr EcsStorageIterator& operator-=(const difference_type Value) noexcept
	{
		Offset += Value;
		return *this;
	}

	constexpr EcsStorageIterator operator-(const difference_type Value) const noexcept
	{
		SparseSetIterator copy = *this;
		return copy -= Value;
	}

	constexpr reference operator[](const difference_type Value) const noexcept
	{
		typename ContainerType::size_type pos = static_cast<typename ContainerType::size_type>(Index() - Value);
		return (*Container)[pos / PageSize][FastMod(pos, PageSize)];
	}

	constexpr reference operator*() const noexcept
	{
		return operator[](0);
	}

	constexpr pointer operator->() const noexcept
	{
		return std::addressof(operator[](0));
	}

	constexpr difference_type Index() const noexcept
	{
		return Offset - 1;
	}

	constexpr auto operator<=>(const EcsStorageIterator& Other) const noexcept
	{
		return Index() <=> Other.Index();
	}

	constexpr bool operator==(const EcsStorageIterator& Other) const noexcept
	{
		return Index() == Other.Index();
	}

private:
	ContainerType* Container;
	difference_type Offset;
};

template <typename ComponentType, typename Entity>
class EcsComponentStorage : public SparseSet<Entity>
{
	using Traits = EcsComponentTraits<ComponentType, Entity>;

public:
	using value_type = ComponentType;
	using base_type = SparseSet<Entity>;
	using size_type = std::size_t;
	using reference = ComponentType&;
	using const_reference = const ComponentType&;
	using difference_type = std::ptrdiff_t;
	using iterator = EcsStorageIterator<std::vector<ComponentType*>, Traits::PageSize>;
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using signal_type = Signal<void(const Entity)>;

	EcsComponentStorage()
		: base_type(base_type::Usage::Component)
	{
	}

	EcsComponentStorage(const EcsComponentStorage&) = delete;

	EcsComponentStorage(EcsComponentStorage&& Other) noexcept
		: base_type(std::move(Other))
		  , ComponentContainer(std::move(Other.ComponentContainer))
		  , AddedSignal(std::move(Other.AddedSignal))
		  , RemovedSignal(std::move(Other.RemovedSignal))
		  , UpdatedSignal(std::move(Other.UpdatedSignal))
	{
	}

	EcsComponentStorage& operator=(const EcsComponentStorage&) = delete;

	EcsComponentStorage& operator=(EcsComponentStorage&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	~EcsComponentStorage() override
	{
		FreeComponentPages();
	}

	void Swap(EcsComponentStorage& Other) noexcept
	{
		std::swap(ComponentContainer, Other.ComponentContainer);
		std::swap(AddedSignal, Other.AddedSignal);
		std::swap(RemovedSignal, Other.RemovedSignal);
		std::swap(UpdatedSignal, Other.UpdatedSignal);
		base_type::Swap(Other);
	}

	void Reserve(const uint64 Count) override
	{
		if (Count == 0)
		{
			return;
		}

		base_type::Reserve(Count);
		GetCreateComponentSlot(Count);
	}

	uint64 Capacity() const noexcept override
	{
		return ComponentContainer.size() * Traits::PageSize;
	}

	ComponentType* Raw() noexcept
	{
		return ComponentContainer.data();
	}

	iterator begin() const noexcept
	{
		const difference_type pos = static_cast<difference_type>(base_type::Count());
		return iterator{&ComponentContainer, pos};
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	iterator end() const noexcept
	{
		return iterator{&ComponentContainer, 0u};
	}

	const_iterator cend() const noexcept
	{
		return end();
	}

	reverse_iterator rbegin() const noexcept
	{
		return std::make_reverse_iterator(begin());
	}

	const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	reverse_iterator rend() const noexcept
	{
		return std::make_reverse_iterator(end());
	}

	const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	const ComponentType& GetComponent(const Entity EcsEntity) const noexcept
	{
		return GetComponentRef(base_type::GetSparseIndex(EcsEntity));
	}

	ComponentType& GetComponent(const Entity EcsEntity) noexcept
	{
		UpdatedSignal.Dispatch(EcsEntity);
		return GetComponentRef(base_type::GetSparseIndex(EcsEntity));
	}

	std::tuple<const ComponentType&> GetComponentAsTuple(const Entity EcsEntity) const noexcept
	{
		return std::forward_as_tuple(GetComponent(EcsEntity));
	}

	std::tuple<ComponentType&> GetComponentAsTuple(const Entity EcsEntity) noexcept
	{
		UpdatedSignal.Dispatch(EcsEntity);
		return std::forward_as_tuple(GetComponent(EcsEntity));
	}

	template <typename... Args>
	ComponentType& CreateComponent(const Entity EcsEntity, Args&&... InArgs)
	{
		const typename base_type::iterator it = CreateComponentImpl(EcsEntity, std::forward<Args>(InArgs)...);
		AddedSignal.Dispatch(EcsEntity);
		return GetComponentRef(it.Index());
	}

	template <typename EntityIterator>
	iterator CreateComponent(EntityIterator FirstEntity, EntityIterator LastEntity, const ComponentType& Component = {})
	{
		for (EntityIterator current = FirstEntity; current != LastEntity; ++current)
		{
			CreateComponent(*current, Component);
		}

		return begin();
	}

	template <typename EntityIterator, typename ComponentIterator, typename = std::enable_if<std::is_same_v<
		          typename std::iterator_traits<ComponentIterator>::value_type, ComponentType>>>
	iterator CreateComponents(EntityIterator FirstEntity, EntityIterator LastEntity, ComponentIterator FirstComponent)
	{
		for (EntityIterator current = FirstEntity; current != LastEntity; ++current, ++FirstComponent)
		{
			CreateComponent(*current, *FirstComponent);
		}

		return begin();
	}

	template <typename... Func>
	ComponentType& RunOnComponent(const Entity EcsEntity, Func&&... InFunc)
	{
		UpdatedSignal.Dispatch(EcsEntity);
		const size_type idx = base_type::GetSparseIndex(EcsEntity);
		ComponentType& component = GetComponentRef(idx);
		(std::forward<Func>(InFunc)(component), ...);
		return component;
	}

	auto GetOnAddedSink() noexcept
	{
		return Sink{ AddedSignal };
	}

	auto GetOnRemovedSink() noexcept
	{
		return Sink{ RemovedSignal };
	}

	auto GetOnUpdatedSink() noexcept
	{
		return Sink{ UpdatedSignal };
	}

protected:
	void Pop(const typename base_type::iterator Begin, const typename base_type::iterator End) override
	{
		for (typename base_type::iterator current = Begin; current != End; ++current)
		{
			RemovedSignal.Dispatch(*current);
			ComponentType& component = GetComponentRef(base_type::GetSparseIndex(*current));
			ComponentType& lastComponent = GetComponentRef(static_cast<size_type>(base_type::Count() - 1));
			std::exchange(component, std::move(lastComponent));
			std::destroy_at(std::addressof(component));
			base_type::SwapPop(current);
		}
	}

	void PopAll() override
	{
		for (typename base_type::iterator current = base_type::begin(); current.Index() >= 0; ++current)
		{
			RemovedSignal.Dispatch(*current);
			base_type::SwapPop(current);
			ComponentType& component = GetComponentRef(current.Index());
			std::destroy_at(std::addressof(component));
		}
	}

private:
	void FreeComponentPages()
	{
		for (ComponentType* page : ComponentContainer)
		{
			delete[] page;
			page = nullptr;
		}
	}

	ComponentType& GetComponentRef(const size_type Position)
	{
		return ComponentContainer[Position / Traits::PageSize][FastMod(Position, Traits::PageSize)];
	}

	ComponentType* GetCreateComponentSlot(const size_type Position)
	{
		const size_type pageIdx = Position / Traits::PageSize;
		if (pageIdx >= ComponentContainer.size())
		{
			size_type current = ComponentContainer.size();
			ComponentContainer.resize(pageIdx + 1, nullptr);
			for (; current < ComponentContainer.size(); ++current)
			{
				ComponentContainer[current] = new ComponentType[Traits::PageSize];
			}
		}

		return ComponentContainer[pageIdx] + FastMod(Position, Traits::PageSize);
	}

	template <typename... Args>
	typename base_type::iterator CreateComponentImpl(const Entity EcsEntity, Args&&... InArgs)
	{
		typename base_type::iterator it = base_type::Add(EcsEntity);
		ComponentType* component = std::to_address(GetCreateComponentSlot(static_cast<size_type>(it.Index())));
		std::uninitialized_construct_using_allocator(component, ComponentContainer.get_allocator(), std::forward<Args>(InArgs)...);

		return it;
	}

private:
	std::vector<ComponentType*> ComponentContainer;
	signal_type AddedSignal;
	signal_type RemovedSignal;
	signal_type UpdatedSignal; // Doesn't handle iterator
};

template <typename Entity>
class EcsEntityStorage : public SparseSet<Entity>
{
	using Traits = EcsTraits<Entity>;

public:
	using base_type = SparseSet<Entity>;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using SparseSet<Entity>::SparseSet;

	EcsEntityStorage()
		: base_type(base_type::Usage::Entity)
		  , EntityCounter{}
	{
	}

	Entity CreateEntity()
	{
		const size_t head = base_type::GetFreeListHead();
		const Entity entity = head == base_type::Count() ? GetAvailableEntity() : base_type::Data()[head];
		return *base_type::TryAdd(entity);
	}

private:
	Entity GetAvailableEntity()
	{
		Entity availableEntity = GetNextEntity();
		while (base_type::GetContainedEntityGeneration(availableEntity) != Traits::GetGeneration(EcsEntityNull)
			&& availableEntity != EcsEntityNull)
		{
			availableEntity = GetNextEntity();
		}

		return availableEntity;
	}

	Entity GetNextEntity()
	{
		const Entity nextEntity = Traits::CreateCombined(static_cast<typename Traits::ValueType>(EntityCounter), {});
		LE_ASSERT_DESC(nextEntity != EcsEntityNull, "Hit the active Entity limit")

		if (nextEntity != EcsEntityNull)
		{
			++EntityCounter;
		}

		return nextEntity;
	}

private:
	typename Traits::IdType EntityCounter;
};
}
