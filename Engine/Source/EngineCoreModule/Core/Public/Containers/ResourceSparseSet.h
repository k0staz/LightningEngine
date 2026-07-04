#pragma once

#include <vector>

#include "CoreDefinitions.h"
#include "CoreConcepts.h"
#include "Misc/Uid.h"

namespace LE
{
template <typename ContainerType, uint64 PageSize>
class ResourceSparseSetIterator
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
	using pointer = typename iterator_traits::pointer;
	using reference = typename iterator_traits::reference;
	using difference_type = typename iterator_traits::difference_type;
	using iterator_category = std::random_access_iterator_tag;

	constexpr ResourceSparseSetIterator() noexcept = default;

	constexpr ResourceSparseSetIterator(const ContainerType* ContainerPtr, const difference_type OffsetIn) noexcept
		: Container(ContainerPtr)
		  , Offset(OffsetIn)
	{
	}

	constexpr ResourceSparseSetIterator& operator++() noexcept
	{
		++Offset;
		return *this;
	}

	constexpr ResourceSparseSetIterator& operator++(int) noexcept
	{
		ResourceSparseSetIterator current = *this;
		operator++();
		return current;
	}

	constexpr ResourceSparseSetIterator& operator--() noexcept
	{
		--Offset;
		return *this;
	}

	constexpr ResourceSparseSetIterator& operator--(int) noexcept
	{
		ResourceSparseSetIterator current = *this;
		operator--();
		return current;
	}

	constexpr ResourceSparseSetIterator& operator+=(const difference_type Value) noexcept
	{
		Offset += Value;
		return *this;
	}

	constexpr ResourceSparseSetIterator operator+(const difference_type Value) const noexcept
	{
		ResourceSparseSetIterator copy = *this;
		return copy += Value;
	}

	constexpr ResourceSparseSetIterator& operator-=(const difference_type Value) noexcept
	{
		Offset -= Value;
		return *this;
	}

	constexpr ResourceSparseSetIterator operator-(const difference_type Value) const noexcept
	{
		ResourceSparseSetIterator copy = *this;
		return copy -= Value;
	}

	constexpr difference_type Index() const noexcept
	{
		return Offset;
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

	constexpr auto operator<=>(const ResourceSparseSetIterator& Other) const noexcept
	{
		return Index() <=> Other.Index();
	}

	constexpr bool operator==(const ResourceSparseSetIterator& Other) const noexcept
	{
		return Index() == Other.Index();
	}

private:
	const ContainerType* Container;
	difference_type Offset;
};

template <Identifier IdType, typename BaseResourceType, typename Traits>
class ResourceSparseSetBase
{
public:
	using size_type = std::size_t;

	ResourceSparseSetBase() = default;

	ResourceSparseSetBase(const ResourceSparseSetBase&) = delete;

	ResourceSparseSetBase(ResourceSparseSetBase&& Other) noexcept
		: Sparse(std::move(Other.Sparse))
		  , ReusableIds(std::move(Other.ReusableIds))
		  , PackedToSparse(std::move(Other.PackedToSparse))
		  , IdCounter(std::move(Other.IdCounter))
	{
	}

	virtual ~ResourceSparseSetBase()
	{
		ReleaseSparsePages();
	}

	ResourceSparseSetBase& operator=(const ResourceSparseSetBase&) = delete;

	ResourceSparseSetBase& operator=(ResourceSparseSetBase&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	void Swap(ResourceSparseSetBase& Other) noexcept
	{
		std::swap(Sparse, Other.Sparse);
		std::swap(ReusableIds, Other.ReusableIds);
		std::swap(PackedToSparse, Other.PackedToSparse);
		std::swap(IdCounter, Other.IdCounter);
	}

	size_type size() const noexcept
	{
		return PackedToSparse.size();
	}

	bool empty() const noexcept
	{
		return PackedToSparse.empty();
	}

	void PopResource(const IdType OuterId)
	{
		SwapPop(OuterId);
	}
	
	template <typename Func, typename... Args>
	void RunOnEach(Func Function, Args&&... Arguments)
	{
		const size_type packedSize = size();
		for (size_type i = 0; i < packedSize; ++i)
		{
			Function(GetResource(i), std::forward<Args>(Arguments)...);
		}
	}

	virtual bool Has(const IdType OuterId) const = 0;

	virtual BaseResourceType& GetResource(const IdType Id) const noexcept = 0;

	virtual BaseResourceType& CreateNewResource() = 0;

	virtual void Clear()
	{
		ReleasePackedResources();
		ReleaseSparsePages();
		PackedToSparse.clear();
	}

protected:
	void ReleaseSparsePages()
	{
		for (auto& page : Sparse)
		{
			free(page);
			page = nullptr;
		}
	}

	virtual void ReleasePackedResources() = 0;

	virtual void SwapPop(const IdType OuterId)
	{
		// Update pointers from sparse to packed
		IdType& swappedSparse = GetSparseRefWithIndex(PackedToSparse.back());
		IdType swappedTemp = swappedSparse;
		IdType& poppedSparse = GetSparseRef(OuterId);

		swappedSparse = Traits::CreateCombined(static_cast<typename Traits::ValueType>(poppedSparse),
			Traits::GetGenerationAsValue(swappedSparse));
		poppedSparse = Traits::CreateCombined(static_cast<typename Traits::ValueType>(swappedTemp),
			Traits::GetGenerationAsValue(poppedSparse));

		// Update packed to sparse
		PackedToSparse[Traits::GetId(swappedSparse)] = PackedToSparse.back();
		PackedToSparse.pop_back();

		// Put popped into reusable array
		ReusableIds.emplace_back(IncrementGeneration(OuterId));
	}

	size_type GetSparsePageIndex(const size_type SparseIndex) const noexcept
	{
		return SparseIndex / static_cast<size_type>(Traits::PageSize);
	}

	size_type GetIndex(const IdType OuterId) const noexcept
	{
		return static_cast<size_type>(Traits::GetId(OuterId));
	}

	size_type GetPackedIndex(const IdType OuterId) const
	{
		LE_ASSERT_DESC(Has(OuterId), "Resource Sparse Set doesn't contain ID: {}", OuterId)
		return Traits::GetId(GetSparseRef(OuterId));
	}

	IdType* GetSparsePointer(const IdType OuterId) const noexcept
	{
		const size_type sparseIndex = GetIndex(OuterId);
		return GetSparsePointerWithIndex(sparseIndex);
	}

	IdType* GetSparsePointerWithIndex(const size_type SparseIndex) const noexcept
	{
		const size_type pageIndex = GetSparsePageIndex(SparseIndex);
		if (pageIndex >= Sparse.size())
		{
			return nullptr;
		}

		IdType* page = Sparse[pageIndex];
		if (!page)
		{
			return nullptr;
		}

		return page + FastMod(SparseIndex, Traits::PageSize);
	}

	typename Traits::GenerationType GetContainedSparseGeneration(const IdType Outer)
	{
		if (const IdType* sparsePtr = GetSparsePointer(Outer))
		{
			return Traits::GetGeneration(*sparsePtr);
		}

		return Traits::GetGeneration(NullId());
	}

	typename Traits::GenerationType UpdateGeneration(const IdType Id)
	{
		IdType& sparseRef = GetSparseRef(Id);
		sparseRef = Traits::CreateCombined(Traits::GetAsValue(sparseRef), Traits::GetGenerationAsValue(Id));
		return Traits::GetGeneration(sparseRef);
	}

	typename Traits::GenerationType IncrementGeneration(const IdType Id)
	{
		const IdType newId = Traits::IncrementGeneration(Id);
		return UpdateGeneration(Id);
	}

	IdType& GetSparseRefWithIndex(const size_type Index) const noexcept
	{
		IdType* sparsePtr = GetSparsePointerWithIndex(Index);
		LE_ASSERT_DESC(sparsePtr, "Failed to get sparse pointer at the index {}", Index)
		return *sparsePtr;
	}

	IdType& GetSparseRef(const IdType OuterId) const noexcept
	{
		IdType* sparsePtr = GetSparsePointer(OuterId);
		LE_ASSERT_DESC(sparsePtr, "Failed to get sparse pointer for the resource with Id: {}", OuterId)
		return *sparsePtr;
	}

	IdType& GetCreateSparseElement(const IdType OuterId)
	{
		const size_type sparseIndex = GetIndex(OuterId);
		const size_type pageIndex = GetSparsePageIndex(sparseIndex);

		if (Sparse.size() <= pageIndex)
		{
			Sparse.resize(pageIndex + 1, nullptr);
		}

		if (!Sparse[pageIndex])
		{
			Sparse[pageIndex] = static_cast<IdType*>(malloc(Traits::PageSize * sizeof(IdType)));
			constexpr IdType nullId = NullId();
			std::uninitialized_fill(Sparse[pageIndex], Sparse[pageIndex] + Traits::PageSize, nullId);
		}

		return Sparse[pageIndex][FastMod(sparseIndex, Traits::PageSize)];
	}

	IdType& TryAdd(const IdType OuterId)
	{
		LE_ASSERT_DESC(OuterId != NullId(), "Invalid resource Id when adding resource")

		IdType& sparseElement = GetCreateSparseElement(OuterId);
		size_type packedIndex = size();

		sparseElement = Traits::CreateCombined(static_cast<typename Traits::ValueType>(packedIndex), Traits::GetGenerationAsValue(OuterId));
		PackedToSparse.emplace_back(GetIndex(OuterId));

		return sparseElement;
	}


	IdType GetAvailableId()
	{
		if (!ReusableIds.empty())
		{
			const IdType newId = ReusableIds.back();
			ReusableIds.pop_back();
			return newId;
		}

		const IdType newId = Traits::CreateCombined(static_cast<typename Traits::ValueType>(IdCounter++), {});
		LE_ASSERT_DESC(newId != NullId(), "Hit the active resource limit of type")
		return newId;
	}

private:
	std::vector<IdType*> Sparse = {}; // Stores position + gen of the Resource in the container
	std::vector<IdType> ReusableIds = {};
	std::vector<size_type> PackedToSparse = {};
	typename Traits::IdType IdCounter = 0;
};

// Storage which contains all objects of one resource type
template <Identifier IdType, typename ResourceType, typename BaseResourceType, typename Traits, typename BaseType = ResourceSparseSetBase<IdType, BaseResourceType, Traits>>
requires DerivedFrom<ResourceType, BaseResourceType>
class ResourceSparseSet : public BaseType
{
public:
	using value_type = ResourceType;
	using base_type = BaseType;
	using size_type = std::size_t;
	using reference = ResourceType&;
	using const_reference = const ResourceType&;
	using difference_type = std::ptrdiff_t;
	using iterator = ResourceSparseSetIterator<std::vector<ResourceType*>, Traits::PageSize>;
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	ResourceSparseSet() = default;

	ResourceSparseSet(const ResourceSparseSet&) = delete;

	ResourceSparseSet(ResourceSparseSet&& Other) noexcept
		: ResourceContainer(std::move(Other.ResourceContainer))
	{
	}


	~ResourceSparseSet() override
	{
		base_type::Clear();
	}

	ResourceSparseSet& operator=(const ResourceSparseSet&) = delete;

	ResourceSparseSet& operator=(ResourceSparseSet&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	void Swap(ResourceSparseSet& Other) noexcept
	{
		base_type::Swap(this, Other);
		std::swap(ResourceContainer, Other.ResourceContainer);
	}

	iterator begin() const noexcept
	{
		return iterator{&ResourceContainer, 0u};
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	iterator end() const noexcept
	{
		const difference_type pos = static_cast<difference_type>(base_type::size());
		return iterator{&ResourceContainer, pos};
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

	BaseResourceType& GetResource(const IdType Id) const noexcept override
	{
		return static_cast<BaseResourceType&>(GetResourceRef(base_type::GetIndex(base_type::GetSparseRef(Id))));
	}

	bool Has(const IdType OuterId) const override
	{
		const IdType* sparsePtr = base_type::GetSparsePointer(OuterId);
		if (!sparsePtr)
		{
			return false;
		}

		constexpr typename Traits::IdType cap = Traits::IdMask;
		constexpr typename Traits::ValueType generationMask = Traits::GetAsValue(NullId()) & ~cap;

		auto nullCheck = generationMask & OuterId;
		auto genCheck = nullCheck ^ *sparsePtr;

		return genCheck < cap;
	}

	const_iterator Find(const IdType OuterId)
	{
		return Has(OuterId) ? GetIterator(OuterId) : end();
	}
	
	BaseResourceType& CreateNewResource() override
	{
		IdType newResourceId = base_type::GetAvailableId();
		IdType& sparse = base_type::TryAdd(newResourceId);
		ResourceType* resource = std::to_address(GetCreateResource(base_type::GetIndex(sparse)));
		std::construct_at(resource);
		return static_cast<BaseResourceType&>(*resource);
	}

protected:
	void ReleasePackedResources() override
	{
		for (iterator current = begin(); current != end(); ++current)
		{
			ResourceType& resource = GetResourceRef(current.Index());
			std::destroy_at(std::addressof(resource));
		}

		for (ResourceType*& page : ResourceContainer)
		{
			free(page);
			page = nullptr;
		}
		ResourceContainer.clear();
	}

protected:
	void SwapPop(const IdType OuterId) override
	{
		// Swap and delete resource
		const size_type packedIdx = base_type::GetIndex(base_type::GetSparseRef(OuterId));
		ResourceType& resource = GetResourceRef(packedIdx);
		ResourceType& lastResource = GetResourceRef(base_type::size() - 1);
		std::exchange(resource, std::move(lastResource));

		// Update sparse array
		base_type::SwapPop(OuterId);

		// Destroy
		std::destroy_at(std::addressof(lastResource));
	}

	iterator GetIterator(const IdType OuterId) const
	{
		return (begin() + static_cast<difference_type>(GetPackedIndex(OuterId)));
	}

	ResourceType& GetResourceRef(const size_type PackedPosition) const
	{
		return ResourceContainer[PackedPosition / Traits::PageSize][FastMod(PackedPosition, Traits::PageSize)];
	}

	ResourceType* GetCreateResource(const size_type Position)
	{
		const size_type pageIdx = Position / Traits::PageSize;
		if (pageIdx >= ResourceContainer.size())
		{
			size_type current = ResourceContainer.size();
			ResourceContainer.resize(pageIdx + 1, nullptr);
			for (; current < ResourceContainer.size(); ++current)
			{
				ResourceContainer[current] = static_cast<ResourceType*>(malloc(Traits::PageSize * sizeof(ResourceType)));
			}
		}
		return ResourceContainer[pageIdx] + FastMod(Position, Traits::PageSize);
	}

private:
	std::vector<ResourceType*> ResourceContainer = {}; // Stores pages of resources
};
}
