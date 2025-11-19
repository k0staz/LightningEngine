#pragma once
#include <unordered_set>

#include "AssetRegistry.h"
#include "AssetStorage.h"
#include "Misc/Paths.h"
#include "Service/ServiceBase.h"
#include "Service/ServiceRegistry.h"

namespace LE
{
template <DerivedFromAsset AssetType>
class AssetHandle
{
public:
	AssetHandle()
		: Id(AssetIdNull)
	{
	}

	AssetHandle(const AssetHandle& OtherAssetHandle)
		: Id(OtherAssetHandle.Id)
	{
		InternalAddRef();
	}

	AssetHandle& operator=(const AssetHandle& OtherAssetHandle)
	{
		if (Id != OtherAssetHandle.Id)
		{
			InternalRelease();
			Id = OtherAssetHandle.Id;
			InternalAddRef();
		}

		return *this;
	}

	AssetHandle(AssetHandle&& OtherAssetHandle) noexcept
		: Id(OtherAssetHandle.Id)
	{
		OtherAssetHandle.Id = AssetIdNull;
	}

	AssetHandle& operator=(AssetHandle&& OtherAssetHandle) noexcept
	{
		if (Id != OtherAssetHandle.Id)
		{
			InternalRelease();
			Id = OtherAssetHandle.Id;
			OtherAssetHandle.Id = AssetIdNull;
		}

		return *this;
	}

	~AssetHandle()
	{
		InternalRelease();
	}

	const AssetType* operator->() const
	{
		return *(GetAssetRef());
	}

	bool operator==(const AssetHandle& Other) const
	{
		return Id == Other.Id;
	}

	bool operator==(const AssetType& AssetObject) const
	{
		return Id == AssetObject.RuntimeId;
	}

	bool IsValid() const;

	const AssetType& GetAssetRef();

private:
	AssetHandle(const AssetType& Asset)
		: Id(Asset.RuntimeId)
	{
		InternalAddRef();
	}

	void InternalAddRef()
	{
		if (GetAssetRef().AddRef() == 1)
		{
			RemoveUnloadRequest();
		}
	}

	void InternalRelease()
	{
		if (GetAssetRef().Release() == 0)
		{
			RequestAssetUnload();
		}
	}

	void RequestAssetUnload();
	void RemoveUnloadRequest();

private:
	AssetId Id;
	friend class AssetManager;

	friend bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, AssetHandle& Value);
	friend bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const AssetHandle& Value);
};

class AssetManager : public ServiceBase
{
	using IdType = AssetId;

public:
	AssetManager() = default;

	void Initialize() override
	{
	}

	void ShutDown() override
	{
	}

	using size_type = std::size_t;

	template <DerivedFromAsset AssetType>
	AssetHandle<AssetType> GetAssetUsingPath(const Path& AssetPath)
	{
		Uid assetUid = GetServiceRegistry().GetService<AssetRegistry>().GetUidFromPath(AssetPath);
		if (!assetUid.IsValid())
		{
			LE_INFO("There is no Asset at the path: {}", AssetPath.generic_string());
			return AssetHandle<AssetType>{};
		}

		return GetAsset<AssetType>(assetUid);
	}

	template <DerivedFromAsset AssetType>
	AssetHandle<AssetType> GetAsset(Uid AssetUid)
	{
		AssetRegistry& assetRegistry = GetServiceRegistry().GetService<AssetRegistry>();
		AssetInfo assetInfo = assetRegistry.GetAssetInfo(AssetUid);
		if (!assetInfo.IsValid())
		{
			LE_INFO("There is no Asset with UID: {}", Uid::ToString(assetInfo.AssetUid));
			return AssetHandle<AssetType>{};
		}

		if (HasAsset<AssetType>(assetInfo.RuntimeId))
		{
			return AssetHandle<AssetType>{GetAssetRef<AssetType>(assetInfo.RuntimeId)};
		}

		AssetStorage<IdType, AssetType>& assetStorage = GetCreateAssetStorage<AssetType>();
		AssetType& newAsset = assetStorage.CreateNewAsset(assetInfo.AssetUid);
		assetRegistry.UpdateAssetRuntimeId(assetInfo.AssetUid, newAsset.GetAssetId());

		return AssetHandle<AssetType>{newAsset};
	}

	template <DerivedFromAsset AssetType>
	void LoadAsset(AssetHandle<AssetType> Handle)
	{
		if (!Handle.IsValid())
		{
			return;
		}

		const AssetType& asset = Handle.GetAssetRef();
		asset.State = AssetState::Loading;

		// TODO:
		// - Registry
		// - Callback
		// - Job
	}

	template <DerivedFromAsset AssetType>
	void LoadAssetAsync(AssetHandle<AssetType> Handle)
	{
		// TODO:
		// - Registry
		// - Callback
		// - Job
	}

	void OnFrameEnd();

private:
	template <DerivedFromAsset AssetType>
	AssetType& GetAssetRef(AssetId Id, AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value) const
	{
		const AssetStorage<IdType, AssetType>& assetStorage = *GetAssetStorage<AssetType>();
		return assetStorage.GetAsset(Id);
	}

	template <DerivedFromAsset AssetType>
	void DequeueAssetUnload(AssetId Id, AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value)
	{
		PendingDelete[TypeId].erase(Id);
	}

	template <DerivedFromAsset AssetType>
	void QueueAssetUnload(AssetId Id, AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value)
	{
		PendingDelete[TypeId].emplace(Id);
	}

	template <DerivedFromAsset AssetType>
	bool HasAsset(AssetId Id, AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value) const
	{
		const AssetStorage<IdType, AssetType>* assetStorage = GetAssetStorage<AssetType>();
		if (!assetStorage)
		{
			return false;
		}

		return assetStorage->Has(Id);
	}

	template <DerivedFromAsset AssetType>
	AssetStorage<IdType, AssetType>& GetCreateAssetStorage(AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value)
	{
		static_assert(!std::is_same_v<AssetType, IdType>, "Attempting to pass Asset Id as Asset Type Id");
		using AssetStorageType = AssetStorage<IdType, AssetType>;

		auto it = AssetStorages.find(TypeId);
		if (it != AssetStorages.end())
		{
			return static_cast<AssetStorageType&>(*it->second);
		}

		std::shared_ptr<AssetStorageType> storage = std::make_shared<AssetStorageType>();
		AssetStorages.emplace(TypeId, storage);

		return static_cast<AssetStorageType&>(*storage);
	}

	template <DerivedFromAsset AssetType>
	const AssetStorage<IdType, AssetType>* GetAssetStorage(AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value) const
	{
		static_assert(!std::is_same_v<AssetType, IdType>, "Attempting to pass Asset Id as Asset Type Id");
		using AssetStorageType = AssetStorage<IdType, AssetType>;

		auto it = AssetStorages.find(TypeId);
		if (it != AssetStorages.end())
		{
			return static_cast<AssetStorageType*>(it->second.get());
		}

		return nullptr;
	}

private:
	std::unordered_map<AssetTypeId, std::shared_ptr<AssetStorageBase<IdType>>> AssetStorages;
	std::unordered_map<AssetTypeId, std::unordered_set<AssetId>> PendingDelete;

	template <DerivedFromAsset AssetType>
	friend class AssetHandle;
};

REGISTER_SERVICE_TYPE(AssetManager, "AssetManager")

template <DerivedFromAsset AssetType>
bool AssetHandle<AssetType>::IsValid() const
{
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	return manager.HasAsset<AssetType>(Id);
}

template <DerivedFromAsset AssetType>
const AssetType& AssetHandle<AssetType>::GetAssetRef()
{
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	return manager.GetAssetRef<AssetType>(Id);
}

template <DerivedFromAsset AssetType>
void AssetHandle<AssetType>::RequestAssetUnload()
{
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	manager.QueueAssetUnload<AssetType>(Id);
}

template <DerivedFromAsset AssetType>
void AssetHandle<AssetType>::RemoveUnloadRequest()
{
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	manager.DequeueAssetUnload<AssetType>(Id);
}

template <DerivedFromAsset AssetType>
bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const AssetHandle<AssetType>& Value)
{
	const Uid refAssetUid = Value->GetStableId();
	if (!Archive::Serialize(Ctx, Writer, refAssetUid))
	{
		Ctx.Error.Desc = "Failed when writing UID for the referenced Asset";
		return false;
	}

	if (!Ctx.Info)
	{
		LE_ASSERT_DESC(false, "You must provide associated Asset Info when serializing Asset Handle")
		return false;
	}

	Ctx.Info->Dependencies.emplace(refAssetUid);
	return true;
}

template <DerivedFromAsset AssetType>
bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, AssetHandle<AssetType>& Value)
{
	Uid refAssetUid;
	if (!Archive::Deserialize(Ctx, Reader, refAssetUid))
	{
		Ctx.Error.Desc = "Failed when reading UID for the referenced Asset";
		return false;
	}

	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	Value = manager.GetAsset<AssetType>(refAssetUid);
	return true;
}
}
