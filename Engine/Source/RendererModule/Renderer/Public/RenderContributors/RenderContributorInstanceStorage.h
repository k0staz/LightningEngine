#pragma once
#include "Containers/ECSStorage.h"
#include "Containers/SparseSet.h"
#include "RenderContributors/RenderContributerCore.h"

namespace LE::Renderer
{
template<typename DynamicDataType, typename Entity>
class RenderContributorInstanceStorage : public SparseSet<Entity>
{
	using Traits = RenderContributorTraits<RenderContributorId>;
public:
	using value_type = DynamicDataType;
	using base_type = SparseSet<Entity>;
	using size_type = std::size_t;
	using reference = DynamicDataType&;
	using const_reference = const DynamicDataType&;
	using difference_type = std::ptrdiff_t;
	using iterator = EcsStorageIterator<std::vector<DynamicDataType*>, Traits::PageSize>;
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	
	RenderContributorInstanceStorage()
		: base_type(base_type::Usage::Component)
	{
	}
	
	RenderContributorInstanceStorage(const RenderContributorInstanceStorage&) = delete;
	
	RenderContributorInstanceStorage(RenderContributorInstanceStorage&& Other) noexcept
		: base_type(std::move(Other))
		  , InstanceContainer(std::move(Other.InstanceContainer))
	{
	}

	RenderContributorInstanceStorage& operator=(const RenderContributorInstanceStorage&) = delete;

	RenderContributorInstanceStorage& operator=(RenderContributorInstanceStorage&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	~RenderContributorInstanceStorage() override
	{
		FreeInstancePages();
	}

	void Swap(RenderContributorInstanceStorage& Other) noexcept
	{
		std::swap(InstanceContainer, Other.InstanceContainer);
		base_type::Swap(Other);
	}

	void Reserve(const uint64 Count) override
	{
		if (Count == 0)
		{
			return;
		}

		base_type::Reserve(Count);
		GetCreateInstanceSlot(Count);
	}
	
	iterator begin() const noexcept
	{
		const difference_type pos = static_cast<difference_type>(base_type::Count());
		return iterator{&InstanceContainer, pos};
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	iterator end() const noexcept
	{
		return iterator{&InstanceContainer, 0u};
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
	
	const DynamicDataType& GetInstance(const Entity EcsEntity) const noexcept
	{
		return GetInstanceRef(base_type::GetSparseIndex(EcsEntity));
	}

	DynamicDataType& GetInstance(const Entity EcsEntity) noexcept
	{
		return GetInstanceRef(base_type::GetSparseIndex(EcsEntity));
	}
	
	template <typename... Args>
	DynamicDataType& CreateInstance(const Entity EcsEntity, Args&&... InArgs)
	{
		const typename base_type::iterator it = CreateInstanceImpl(EcsEntity, std::forward<Args>(InArgs)...);
		return GetInstanceRef(it.Index());
	}

	void ClearEntries()
	{
		PopAll();
	}
	
protected:
	void Pop(const typename base_type::iterator Begin, const typename base_type::iterator End) override
	{
		for (typename base_type::iterator current = Begin; current != End; ++current)
		{
			DynamicDataType& instance = GetInstanceRef(base_type::GetSparseIndex(*current));
			DynamicDataType& lastInstance = GetInstanceRef(static_cast<size_type>(base_type::Count() - 1));
			std::exchange(instance, std::move(lastInstance));
			std::destroy_at(std::addressof(instance));
			base_type::SwapPop(current);
		}
	}

	void PopAll() override
	{
		for (typename base_type::iterator current = base_type::begin(); current.Index() >= 0; ++current)
		{
			base_type::SwapPop(current);
			DynamicDataType& instance = GetInstanceRef(current.Index());
			std::destroy_at(std::addressof(instance));
		}
	}
	
private:
	void FreeInstancePages()
	{
		for (DynamicDataType* page : InstanceContainer)
		{
			delete[] page;
			page = nullptr;
		}
		InstanceContainer.clear();
	}
	
	DynamicDataType& GetInstanceRef(const size_type Position)
	{
		return InstanceContainer[Position / Traits::PageSize][FastMod(Position, Traits::PageSize)];
	}

	DynamicDataType* GetCreateInstanceSlot(const size_type Position)
	{
		const size_type pageIdx = Position / Traits::PageSize;
		if (pageIdx >= InstanceContainer.size())
		{
			size_type current = InstanceContainer.size();
			InstanceContainer.resize(pageIdx + 1, nullptr);
			for (; current < InstanceContainer.size(); ++current)
			{
				InstanceContainer[current] = new DynamicDataType[Traits::PageSize];
			}
		}

		return InstanceContainer[pageIdx] + FastMod(Position, Traits::PageSize);
	}

	template <typename... Args>
	typename base_type::iterator CreateInstanceImpl(const Entity EcsEntity, Args&&... InArgs)
	{
		typename base_type::iterator it = base_type::Add(EcsEntity);
		DynamicDataType* instance = std::to_address(GetCreateInstanceSlot(static_cast<size_type>(it.Index())));
		std::uninitialized_construct_using_allocator(instance, InstanceContainer.get_allocator(), std::forward<Args>(InArgs)...);

		return it;
	}
private:
	std::vector<DynamicDataType*> InstanceContainer;
};
}
