#pragma once

#include <vector>

#include "Asset.h"
#include "FileManager/FileManager.h"

namespace LE
{
template <typename ContainerType, uint64 PageSize>
class AssetStorageIterator
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

	constexpr AssetStorageIterator() noexcept = default;

	constexpr AssetStorageIterator(ContainerType* ContainerPtr, const difference_type OffsetIn) noexcept
		: Container(ContainerPtr)
		  , Offset(OffsetIn)
	{
	}

	constexpr AssetStorageIterator& operator++() noexcept
	{
		++Offset;
		return *this;
	}

	constexpr AssetStorageIterator& operator++(int) noexcept
	{
		AssetStorageIterator current = *this;
		operator++();
		return current;
	}

	constexpr AssetStorageIterator& operator--() noexcept
	{
		--Offset;
		return *this;
	}

	constexpr AssetStorageIterator& operator--(int) noexcept
	{
		AssetStorageIterator current = *this;
		operator--();
		return current;
	}

	constexpr AssetStorageIterator& operator+=(const difference_type Value) noexcept
	{
		Offset += Value;
		return *this;
	}

	constexpr AssetStorageIterator operator+(const difference_type Value) const noexcept
	{
		AssetStorageIterator copy = *this;
		return copy += Value;
	}

	constexpr AssetStorageIterator& operator-=(const difference_type Value) noexcept
	{
		Offset -= Value;
		return *this;
	}

	constexpr AssetStorageIterator operator-(const difference_type Value) const noexcept
	{
		AssetStorageIterator copy = *this;
		return copy -= Value;
	}

	constexpr difference_type Index() const noexcept
	{
		return Offset - 1;
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

	constexpr auto operator<=>(const AssetStorageIterator& Other) const noexcept
	{
		return Index() <=> Other.Index();
	}

	constexpr bool operator==(const AssetStorageIterator& Other) const noexcept
	{
		return Index() == Other.Index();
	}

private:
	ContainerType* Container;
	difference_type Offset;
};

template <Identifier IdType>
class AssetStorageBase
{
	using Traits = AssetTraits<IdType>;

public:
	using size_type = std::size_t;

	AssetStorageBase() = default;

	AssetStorageBase(const AssetStorageBase&) = delete;

	AssetStorageBase(AssetStorageBase&& Other) noexcept
		: Sparse(std::move(Other.Sparse))
		  , ReusableIds(std::move(Other.ReusableIds))
		  , PackedToSparse(std::move(Other.PackedToSparse))
		  , IdCounter(std::move(Other.IdCounter))
	{
	}

	virtual ~AssetStorageBase()
	{
		ReleaseSparsePages();
	}

	AssetStorageBase& operator=(const AssetStorageBase&) = delete;

	AssetStorageBase& operator=(AssetStorageBase&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	void Swap(AssetStorageBase& Other) noexcept
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

	void PopAsset(const IdType OuterId)
	{
		SwapPop(OuterId);
	}

	virtual bool Has(IdType) const = 0;

	virtual Asset& GetAsset(const IdType Id) const noexcept = 0;

	virtual Asset& CreateNewAsset(Uid StableAssetId) = 0;

	virtual void LoadAsset(AssetInfo& Info) const = 0;

protected:
	void ReleaseSparsePages()
	{
		for (auto& page : Sparse)
		{
			free(page);
			page = nullptr;
		}
	}

	virtual void SwapPop(const IdType OuterId)
	{
		// Update pointers from sparse to packed
		IdType& swappedSparse = GetSparseRefWithIndex(PackedToSparse.back());
		IdType& poppedSparse = GetSparseRef(OuterId);

		swappedSparse = Traits::CreateCombined(static_cast<typename Traits::ValueType>(poppedSparse),
			Traits::GetGenerationAsValue(swappedSparse));
		poppedSparse = Traits::CreateCombined(static_cast<typename Traits::ValueType>(swappedSparse),
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
		LE_ASSERT_DESC(Has(OuterId), "Asset storage doesn't contain ID: {}", OuterId)
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

		return page += FastMod(SparseIndex, Traits::PageSize);
	}

	typename Traits::GenerationType GetContainedSparseGeneration(const IdType Outer)
	{
		if (const IdType* sparsePtr = GetSparsePointer(Outer))
		{
			return Traits::GetGeneration(*sparsePtr);
		}

		return Traits::GetGeneration(AssetIdNull);
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
		LE_ASSERT_DESC(sparsePtr, "Failed to get sparse pointer for the asset with Id: {}", OuterId)
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
			constexpr IdType nullId = AssetIdNull;
			std::uninitialized_fill(Sparse[pageIndex], Sparse[pageIndex] + Traits::PageSize, nullId);
		}

		return Sparse[pageIndex][FastMod(sparseIndex, Traits::PageSize)];
	}

	IdType& TryAdd(const IdType OuterId)
	{
		LE_ASSERT_DESC(OuterId != AssetIdNull, "Invalid asset Id when creating adding asset")

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
		LE_ASSERT_DESC(newId != AssetIdNull, "Hit the active asset limit of type")
		return newId;
	}

private:
	std::vector<IdType*> Sparse = {}; // Stores position + gen of the Asset in the container
	std::vector<IdType> ReusableIds = {};
	std::vector<size_type> PackedToSparse = {};
	typename Traits::IdType IdCounter = 0;
};

// Storage which contains all objects of one asset type
template <Identifier IdType, DerivedFromAsset AssetType>
class AssetStorage : public AssetStorageBase<IdType>
{
	using Traits = AssetTraits<IdType>;

public:
	using value_type = AssetType;
	using base_type = AssetStorageBase<IdType>;
	using size_type = std::size_t;
	using reference = AssetType&;
	using const_reference = const AssetType&;
	using difference_type = std::ptrdiff_t;
	using iterator = AssetStorageIterator<std::vector<AssetType*>, Traits::PageSize>;
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	AssetStorage() = default;

	AssetStorage(const AssetStorage&) = delete;

	AssetStorage(AssetStorage&& Other) noexcept
		: AssetContainer(std::move(Other.AssetContainer))
	{
	}


	~AssetStorage() override
	{
		ReleaseAssets();
	}

	AssetStorage& operator=(const AssetStorage&) = delete;

	AssetStorage& operator=(AssetStorage&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	void Swap(AssetStorage& Other) noexcept
	{
		base_type::Swap(this, Other);
		std::swap(AssetContainer, Other.AssetContainer);
	}

	iterator begin() const noexcept
	{
		return iterator{&AssetContainer, 0u};
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	iterator end() const noexcept
	{
		const difference_type pos = static_cast<difference_type>(base_type::size());
		return iterator{&AssetContainer, pos};
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

	Asset& GetAsset(const IdType Id) const noexcept override
	{
		return static_cast<Asset&>(GetAssetRef(base_type::GetIndex(base_type::GetSparseRef(Id))));
	}

	bool Has(const IdType OuterId) const noexcept override
	{
		const IdType* sparsePtr = base_type::GetSparsePointer(OuterId);
		if (!sparsePtr)
		{
			return false;
		}

		constexpr typename Traits::IdType cap = Traits::IdMask;
		constexpr typename Traits::ValueType generationMask = Traits::GetAsValue(AssetIdNull) & ~cap;

		auto nullCheck = generationMask & OuterId;
		auto genCheck = nullCheck ^ *sparsePtr;

		return genCheck < cap;
	}

	const_iterator Find(const IdType OuterId)
	{
		return Has(OuterId) ? GetIterator(OuterId) : end();
	}

	Asset& CreateNewAsset(Uid StableAssetId) override
	{
		IdType newAssetId = base_type::GetAvailableId();
		IdType& sparse = base_type::TryAdd(newAssetId);
		AssetType* asset = std::to_address(GetCreateAsset(base_type::GetIndex(sparse)));
		std::construct_at(asset, newAssetId, StableAssetId, AssetTypeIdGetter<AssetType>::Value);
		return static_cast<Asset&>(*asset);
	}

	void LoadAsset(AssetInfo& Info) const override
	{
		AssetType& asset = static_cast<AssetType&>(GetAsset(Info.RuntimeId));

		std::vector<std::byte> assetData;
		bool success = LoadFile(Info.PathToAsset, assetData);

		if(success)
		{
			Archive::Context context(&Info);
			Archive::ArchiveReader reader(assetData);

			success = Archive::Deserialize(context, reader, asset);
		}
		
		asset.SetState(success ? AssetState::Loaded : AssetState::FailedLoad);
	}

protected:
	void ReleaseAssets()
	{
		for (AssetType* page : AssetContainer)
		{
			delete[] page;
			page = nullptr;
		}
	}

protected:
	void SwapPop(const IdType OuterId) override
	{
		// Swap and delete asset
		const size_type packedIdx = base_type::GetIndex(base_type::GetSparseRef(OuterId));
		AssetType& asset = GetAssetRef(packedIdx);
		AssetType& lastAsset = GetAssetRef(base_type::size() - 1);
		std::exchange(asset, std::move(lastAsset));
		std::destroy_at(std::addressof(asset));

		base_type::SwapPop(OuterId);
	}

private:
	iterator GetIterator(const IdType OuterId) const
	{
		return (begin() + static_cast<difference_type>(GetPackedIndex(OuterId)));
	}

	AssetType& GetAssetRef(const size_type PackedPosition) const
	{
		return AssetContainer[PackedPosition / Traits::PageSize][FastMod(PackedPosition, Traits::PageSize)];
	}

	AssetType* GetCreateAsset(const size_type Position)
	{
		const size_type pageIdx = Position / Traits::PageSize;
		if (pageIdx >= AssetContainer.size())
		{
			size_type current = AssetContainer.size();
			AssetContainer.resize(pageIdx + 1, nullptr);
			for (; current < AssetContainer.size(); ++current)
			{
				AssetContainer[current] = new AssetType[Traits::PageSize];
			}
		}
		return AssetContainer[pageIdx] + FastMod(Position, Traits::PageSize);
	}

private:
	std::vector<AssetType*> AssetContainer = {}; // Stores pages of assets
};
}
