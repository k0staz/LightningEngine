#pragma once
#include "CoreMinimum.h"
#include "CoreConcepts.h"
#include "Math/Math.h"


namespace LE
{
template <typename PackedContainer>
struct SparseSetIterator
{
	using value_type = typename PackedContainer::value_type;
	using pointer = typename PackedContainer::const_pointer;
	using reference = typename PackedContainer::const_reference;
	using difference_type = typename PackedContainer::difference_type;
	using iterator_category = std::random_access_iterator_tag;

	constexpr SparseSetIterator() noexcept
		: Packed(),
		  Offset()
	{
	}

	constexpr SparseSetIterator(const PackedContainer& InContainer, const difference_type InOffset) noexcept
		: Packed(&InContainer)
		  , Offset(InOffset)
	{
	}

	constexpr SparseSetIterator& operator++() noexcept
	{
		--Offset;
		return *this;
	}

	constexpr SparseSetIterator operator++(int) noexcept
	{
		SparseSetIterator current = *this;
		operator++();
		return current;
	}

	constexpr SparseSetIterator& operator--() noexcept
	{
		++Offset;
		return *this;
	}

	constexpr SparseSetIterator operator--(int) noexcept
	{
		SparseSetIterator current = *this;
		operator--();
		return current;
	}

	constexpr SparseSetIterator& operator+=(const difference_type Value) noexcept
	{
		Offset -= Value;
		return *this;
	}

	constexpr SparseSetIterator operator+(const difference_type Value) const noexcept
	{
		SparseSetIterator copy = *this;
		return copy += Value;
	}

	constexpr SparseSetIterator& operator-=(const difference_type Value) noexcept
	{
		Offset += Value;
		return *this;
	}

	constexpr SparseSetIterator operator-(const difference_type Value) const noexcept
	{
		SparseSetIterator copy = *this;
		return copy -= Value;
	}

	constexpr reference operator[](const difference_type Value) const noexcept
	{
		return (*Packed)[static_cast<typename PackedContainer::size_type>(Index() - Value)];
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

	constexpr auto operator<=>(const SparseSetIterator& Other) const noexcept
	{
		return Index() <=> Other.Index();
	}

	constexpr bool operator==(const SparseSetIterator& Other) const noexcept
	{
		return Index() == Other.Index();
	}

	constexpr bool operator!=(const SparseSetIterator& Other) const noexcept
	{
		return Index() != Other.Index();
	}

private:
	const PackedContainer* Packed;
	difference_type Offset;
};

template <typename PackedContainer>
constexpr std::ptrdiff_t operator-(const SparseSetIterator<PackedContainer>& Lhs, const SparseSetIterator<PackedContainer>& Rhs) noexcept
{
	return Rhs.Index() - Lhs.Index();
}

template <EcsEntityIdentifier Type>
class SparseSet
{
	using Traits = EcsTraits<Type>;
	static constexpr std::size_t MaxSize = static_cast<std::size_t>(Traits::GetId(EcsEntityNull));

public:
	using value_type = Type;
	using size_type = std::size_t;
	using reference = Type&;
	using const_reference = const Type&;
	using difference_type = std::ptrdiff_t;
	using iterator = SparseSetIterator<std::vector<Type>>;
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	enum class Usage : uint8_t
	{
		Component = 0,
		Entity,
	};

	SparseSet(Usage Usage)
		: Sparse({})
		  , Packed({})
		  , CurrentUsage(Usage)
		  , Head(GetUsageHead())
	{
	}

	SparseSet(const SparseSet&) = delete;

	SparseSet(SparseSet&& Other) noexcept
		: Sparse(std::move(Other.Sparse))
		  , Packed(std::move(Other.Packed))
		  , CurrentUsage(Other.CurrentUsage)
		  , Head(std::exchange(Other.Head, GetUsageHead()))
	{
	}

	SparseSet& operator=(const SparseSet&) = delete;

	SparseSet& operator=(SparseSet&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	virtual ~SparseSet()
	{
		ReleaseSparsePages();
	}

	void Swap(SparseSet& Other) noexcept
	{
		std::swap(Sparse, Other.Sparse);
		std::swap(Packed, Other.Packed);
		std::swap(CurrentUsage, Other.CurrentUsage);
		std::swap(Head, Other.Head);
	}

	virtual void Reserve(const uint64 Count)
	{
		Packed.reserve(static_cast<size_t>(Count));
	}

	virtual uint64 Capacity() const noexcept
	{
		return static_cast<uint64>(Packed.capacity());
	}

	uint64 SparseSize() const noexcept
	{
		return static_cast<uint64>(Sparse.size()) * Traits::PageSize;
	}

	uint64 Count() const noexcept
	{
		return static_cast<uint64>(Packed.size());
	}

	bool Empty() const noexcept
	{
		return Packed.empty();
	}

	const Type* Data() const noexcept
	{
		return Packed.data();
	}

	Usage GetUsage() const noexcept
	{
		return CurrentUsage;
	}

	size_type GetFreeListHead() const noexcept
	{
		return Head;
	}

	iterator begin() const noexcept
	{
		const difference_type pos = static_cast<difference_type>(Packed.size());
		return iterator{Packed, pos};
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	iterator end() const noexcept
	{
		return iterator{Packed, {}};
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

	iterator Add(const Type Entity)
	{
		return TryAdd(Entity);
	}

	template <typename Iterator>
	iterator AddRange(const Iterator& Begin, const Iterator& End)
	{
		iterator result = end();
		for (Iterator current = Begin; current != End; ++current)
		{
			TryAdd(*current);
		}

		return result;
	}

	void Delete(const Type Entity)
	{
		const iterator it = GetIterator(Entity);
		Pop(it, it + 1);
	}

	void Clear()
	{
		PopAll();
		Head = GetUsageHead();
	}

	bool Has(const Type Entity) const noexcept
	{
		const Type* sparsePtr = GetSparsePointer(Entity);
		if (!sparsePtr)
		{
			return false;
		}

		constexpr typename Traits::IdType cap = Traits::IdMask; // Lower bits
		constexpr typename Traits::ValueType generationMask = Traits::GetAsValue(EcsEntityNull) & ~cap; // To avoid shifting

		auto nullCheck = generationMask & Entity;
		auto genCheck = nullCheck ^ *sparsePtr;

		return genCheck < cap; // Checks that Generation bits are the same, and that the entity is not null
	}

	const_iterator Find(const Type Entity)
	{
		return Has(Entity) ? GetIterator(Entity) : end();
	}

	size_type GetSparseIndex(const Type Entity) const
	{
		LE_ASSERT_DESC(Has(Entity), "Sparse Set doesn't have provided Entity")

		return GetEntityIndex(GetSparseRef(Entity));
	}

	typename Traits::GenerationType GetContainedEntityGeneration(const Type Entity)
	{
		if (const Type* sparsePtr = GetSparsePointer(Entity))
		{
			return Traits::GetGeneration(*sparsePtr);
		}

		return Traits::GetGeneration(EcsEntityNull);
	}

	typename Traits::GenerationType UpdateGeneration(const Type NewGenEntity)
	{
		Type& sparseRef = GetSparseRef(NewGenEntity);
		sparseRef = Traits::CreateCombined(Traits::GetAsValue(sparseRef), Traits::GetGenerationAsValue(NewGenEntity));
		Packed[Traits::GetId(sparseRef)] = NewGenEntity;
		return Traits::GetGeneration(NewGenEntity);
	}

	typename Traits::GenerationType IncrementGeneration(const Type Entity)
	{
		const Type newGenEntity = Traits::IncrementGeneration(Entity);
		return UpdateGeneration(newGenEntity);
	}

protected:
	virtual iterator TryAdd(const Type Entity)
	{
		LE_ASSERT_DESC(Entity != EcsEntityNull, "Invalid Entity")

		Type& sparseElement = GetCreateSparseElement(Entity);
		size_type packedIndex = static_cast<size_type>(Count());

		if (sparseElement == EcsEntityNull || CurrentUsage == Usage::Component)
		{
			LE_ASSERT_DESC(sparseElement == EcsEntityNull, "Slot is occupied")
			Packed.push_back(Entity);
			sparseElement = Traits::CreateCombined(static_cast<typename Traits::ValueType>(packedIndex), Traits::GetGenerationAsValue(Entity));
		}
		else
		{
			LE_ASSERT_DESC(Traits::GetId(sparseElement) < Head, "Slot is occupied")
			UpdateGeneration(Entity);
		}

		if (CurrentUsage == Usage::Entity)
		{
			packedIndex = Head++;
			SwapAt(Traits::GetId(sparseElement), packedIndex);
		}

		return --(end() - static_cast<difference_type>(packedIndex));
	}

	virtual void Pop(const iterator Begin, const iterator End)
	{
		switch (CurrentUsage)
		{
		case Usage::Component:
			for (iterator current = Begin; current != End; ++current)
			{
				SwapPop(current);
			}
			break;
		case Usage::Entity:
			for (iterator current = Begin; current != End; ++current)
			{
				SwapOnly(current);
			}
			break;
		}
		
	}

	virtual void PopAll()
	{
		for (Type& entity : Packed)
		{
			GetSparseRef(entity) = EcsEntityNull;
		}
		Head = GetUsageHead();
		Packed.clear();
	}

	void SwapPop(const iterator Iterator)
	{
		LE_ASSERT_DESC(CurrentUsage == Usage::Component, "Wrong method for current usage")
		Type& entityToDelete = GetSparseRef(*Iterator);
		const typename Traits::IdType deletePos = Traits::GetId(entityToDelete);

		// Swap Entities
		Type& swapEntity = GetSparseRef(Packed.back());
		if (swapEntity != entityToDelete)
		{
			swapEntity = Traits::CreateCombined(deletePos, Traits::GetGenerationAsValue(Packed.back())); // Swap Entities
			Packed[deletePos] = Packed.back();
			Packed.back() = EcsEntityNull;
			entityToDelete = EcsEntityNull;
		}

		Packed.pop_back();
	}

	void SwapOnly(const iterator Iterator)
	{
		LE_ASSERT_DESC(CurrentUsage == Usage::Entity, "Wrong method for current usage")
		const size_type idx = GetSparseIndex(*Iterator);
		IncrementGeneration(*Iterator);

		Head -= (idx < Head);
		SwapAt(idx, Head);
	}

	void ReleaseSparsePages()
	{
		for (auto& page : Sparse)
		{
			free(page);
			page = nullptr;
		}
	}

	size_type GetSparsePageIndex(const size_type SparseIndex) const noexcept
	{
		return SparseIndex / static_cast<size_type>(Traits::PageSize);
	}

	size_type GetEntityIndex(const Type Entity) const noexcept
	{
		return static_cast<size_type>(Traits::GetId(Entity));
	}

	Type* GetSparsePointer(const Type Entity) const noexcept
	{
		const size_type sparseIndex = GetEntityIndex(Entity);
		const size_type pageIndex = GetSparsePageIndex(sparseIndex);
		if (pageIndex >= Sparse.size())
		{
			return nullptr;
		}

		Type* page = Sparse[pageIndex];
		if (!page)
		{
			return nullptr;
		}

		return page + FastMod(sparseIndex, Traits::PageSize);
	}

	Type& GetSparseRef(const Type Entity) const noexcept
	{
		Type* sparsePtr = GetSparsePointer(Entity);
		LE_ASSERT_DESC(sparsePtr, "Invalid Element")
		return *sparsePtr;
	}

	iterator GetIterator(const Type Entity) const
	{
		return --(end() - static_cast<difference_type>(GetSparseIndex(Entity)));
	}

	void SwapAt(const size_type Lhs, const size_type Rhs)
	{
		Type& from = Packed[Lhs];
		Type& to = Packed[Rhs];

		GetSparseRef(from) = Traits::CreateCombined(static_cast<typename Traits::ValueType>(Rhs), Traits::GetGenerationAsValue(from));
		GetSparseRef(to) = Traits::CreateCombined(static_cast<typename Traits::ValueType>(Lhs), Traits::GetGenerationAsValue(to));

		std::swap(from, to);
	}

	Type& GetCreateSparseElement(const Type Entity)
	{
		const size_type sparseIndex = GetEntityIndex(Entity);
		const size_type pageIndex = GetSparsePageIndex(sparseIndex);
		if (Sparse.size() <= pageIndex)
		{
			Sparse.resize(pageIndex + 1, nullptr);
		}

		if (!Sparse[pageIndex])
		{
			Sparse[pageIndex] = static_cast<Type*>(malloc(Traits::PageSize * sizeof(Type)));

			constexpr Type nullEntity = EcsEntityNull;
			std::uninitialized_fill(Sparse[pageIndex], Sparse[pageIndex] + Traits::PageSize, nullEntity);
		}

		return Sparse[pageIndex][FastMod(sparseIndex, Traits::PageSize)];
	}

	size_type GetUsageHead() const noexcept
	{
		return MaxSize * static_cast<size_type>(CurrentUsage != Usage::Entity);
	}

private:
	std::vector<Type*> Sparse;
	std::vector<Type> Packed;
	Usage CurrentUsage;
	size_type Head;
};
}
