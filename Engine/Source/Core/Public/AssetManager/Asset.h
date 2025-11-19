#pragma once

#include "AssetRegistry.h"
#include "Archive/Archive.h"
#include "AssetManager/AssetCore.h"
#include "Misc/Uid.h"

namespace LE
{
class Asset
{
public:
	Asset(const Asset&) = delete;
	Asset& operator=(const Asset&) = delete;

	Asset() = default;

	Asset(AssetId Id)
		: RuntimeId(Id)
	{
	}

	Asset(AssetId Id, Uid AssetUid, AssetTypeId AssetTypeId)
		: RuntimeId(Id)
		  , StableId(AssetUid)
		  , TypeId(AssetTypeId)
	{
	}

	Asset(Asset&& Other) noexcept
		: RuntimeId(Other.RuntimeId)
		  , StableId(Other.StableId)
		  , TypeId(Other.TypeId)
		  , State(Other.State.load(std::memory_order_relaxed))
		  , RefsNum(Other.RefsNum.load(std::memory_order_relaxed))
	{
	}

	Asset& operator=(Asset&& Other) noexcept
	{
		Swap(Other);
		return *this;
	}

	virtual ~Asset() = default;

	void Swap(Asset& Other) noexcept
	{
		using std::swap;
		swap(RuntimeId, Other.RuntimeId);
		swap(StableId, Other.StableId);
		swap(TypeId, Other.TypeId);

		const AssetState state = State.load(std::memory_order_acquire);
		const AssetState otherState = Other.State.load(std::memory_order_acquire);
		State.store(otherState, std::memory_order_release);
		Other.State.store(state, std::memory_order_release);

		const auto refNum = RefsNum.load(std::memory_order_acquire);
		const auto otherRefNum = Other.RefsNum.load(std::memory_order_acquire);
		RefsNum.store(otherRefNum, std::memory_order_release);
		Other.RefsNum.store(refNum, std::memory_order_release);
	}

	AssetId GetAssetId() const
	{
		return RuntimeId;
	}

	Uid GetStableId() const
	{
		return StableId;
	}

	AssetState GetAssetState() const
	{
		return State.load(std::memory_order_acquire);
	}

	template <class AssetType>
	bool IsOfType() const
	{
		return AssetTypeIdGetter<AssetType>::Value == TypeId;
	}

protected:
	uint32 AddRef() const;
	uint32 Release() const;

	uint32 GetRefCount() const
	{
		return RefsNum.load(std::memory_order_acquire);
	}

private:
	AssetId RuntimeId = AssetIdNull;
	Uid StableId = EmptyUid;
	AssetTypeId TypeId = 0;
	mutable std::atomic<AssetState> State = AssetState::Uninitialized;
	mutable std::atomic_uint RefsNum = 0;

	template <DerivedFromAsset AssetType>
	friend class AssetHandle;

	friend class AssetManager;

	friend bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Archive, Asset& Value);
	friend bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Archive, const Asset& Value);
};

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const Asset& Value)
{
	using namespace LE::Archive;
	// Uid
	if (!Serialize(Ctx, Writer, Value.StableId))
	{
		return false;
	}

	// Type
	if (!Serialize(Ctx, Writer, Value.TypeId))
	{
		return false;
	}

	if (!Ctx.Info)
	{
		LE_ASSERT_DESC(false, "You must provide associated Asset Info when serializing Asset")
		return false;
	}

	Ctx.Info->AssetUid = Value.StableId;

	return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, Asset& Value)
{
	using namespace LE::Archive;
	// Uid
	if (!Deserialize(Ctx, Reader, Value.StableId))
	{
		return false;
	}

	// Type
	if (!Deserialize(Ctx, Reader, Value.TypeId))
	{
		return false;
	}

	return true;
}


}
