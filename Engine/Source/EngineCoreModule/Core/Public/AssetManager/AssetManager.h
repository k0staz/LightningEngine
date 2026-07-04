#pragma once
#include <unordered_set>

#include "AssetDependencyLoaderResolver.h"
#include "AssetRegistry.h"
#include "AssetStorage.h"
#include "AssetStorageFactory.h"
#include "Misc/Paths.h"
#include "Service/ServiceBase.h"
#include "Service/ServiceRegistry.h"

namespace LE
{
template <DerivedFromAsset AssetType = Asset>
class AssetHandle
{
public:
	AssetHandle()
		: Id(AssetIdNull)
		  , UnderlyingTypeId(AssetTypeIdNull)
	{
	}

	AssetHandle(const AssetHandle& OtherAssetHandle)
		: Id(OtherAssetHandle.Id)
		  , UnderlyingTypeId(OtherAssetHandle.UnderlyingTypeId)
	{
		InternalAddRef();
	}

	template <DerivedFromAsset OtherAssetType>
	AssetHandle(const AssetHandle<OtherAssetType>& OtherAssetHandle)
		: Id(OtherAssetHandle.Id)
		  , UnderlyingTypeId(OtherAssetHandle.UnderlyingTypeId)
	{
		static_assert(std::is_base_of_v<OtherAssetType, AssetType>, "AssetType must be derived from OtherAssetType");
		InternalAddRef();
	}

	AssetHandle& operator=(const AssetHandle& OtherAssetHandle)
	{
		if (Id != OtherAssetHandle.Id || UnderlyingTypeId != OtherAssetHandle.UnderlyingTypeId)
		{
			InternalRelease();
			Id = OtherAssetHandle.Id;
			UnderlyingTypeId = OtherAssetHandle.UnderlyingTypeId;
			InternalAddRef();
		}

		return *this;
	}

	template <DerivedFromAsset OtherAssetType>
	AssetHandle& operator=(const AssetHandle<OtherAssetType>& OtherAssetHandle)
	{
		static_assert(std::is_base_of_v<OtherAssetType, AssetType>, "AssetType must be derived from OtherAssetType");
		if (Id != OtherAssetHandle.Id || UnderlyingTypeId != OtherAssetHandle.UnderlyingTypeId)
		{
			InternalRelease();
			Id = OtherAssetHandle.Id;
			UnderlyingTypeId = OtherAssetHandle.UnderlyingTypeId;
			InternalAddRef();
		}

		return *this;
	}

	AssetHandle(AssetHandle&& OtherAssetHandle) noexcept
		: Id(OtherAssetHandle.Id)
		  , UnderlyingTypeId(OtherAssetHandle.UnderlyingTypeId)
	{
		OtherAssetHandle.Id = AssetIdNull;
		OtherAssetHandle.UnderlyingTypeId = AssetTypeIdNull;
	}

	template <DerivedFromAsset OtherAssetType>
	AssetHandle(AssetHandle<OtherAssetType>&& OtherAssetHandle) noexcept
		: Id(OtherAssetHandle.Id)
		  , UnderlyingTypeId(OtherAssetHandle.UnderlyingTypeId)
	{
		static_assert(std::is_base_of_v<OtherAssetType, AssetType>, "AssetType must be derived from OtherAssetType");
		OtherAssetHandle.Id = AssetIdNull;
		OtherAssetHandle.UnderlyingTypeId = AssetTypeIdNull;
	}

	AssetHandle& operator=(AssetHandle&& OtherAssetHandle) noexcept
	{
		if (Id != OtherAssetHandle.Id || UnderlyingTypeId != OtherAssetHandle.UnderlyingTypeId)
		{
			InternalRelease();
			Id = OtherAssetHandle.Id;
			UnderlyingTypeId = OtherAssetHandle.UnderlyingTypeId;
			OtherAssetHandle.Id = AssetIdNull;
			OtherAssetHandle.UnderlyingTypeId = AssetTypeIdNull;
		}

		return *this;
	}

	template <DerivedFromAsset OtherAssetType>
	AssetHandle& operator=(AssetHandle<OtherAssetType>&& OtherAssetHandle) noexcept
	{
		static_assert(std::is_base_of_v<OtherAssetType, AssetType>, "AssetType must be derived from OtherAssetType");
		if (Id != OtherAssetHandle.Id || UnderlyingTypeId != OtherAssetHandle.UnderlyingTypeId)
		{
			InternalRelease();
			Id = OtherAssetHandle.Id;
			UnderlyingTypeId = OtherAssetHandle.UnderlyingTypeId;
			OtherAssetHandle.Id = AssetIdNull;
			OtherAssetHandle.UnderlyingTypeId = AssetTypeIdNull;
		}

		return *this;
	}

	~AssetHandle()
	{
		InternalRelease();
	}

	const AssetType* operator->() const
	{
		return &(GetAssetRef());
	}

	bool operator==(const AssetHandle& Other) const
	{
		return Id == Other.Id && UnderlyingTypeId != Other.UnderlyingTypeId;
	}

	bool operator==(const AssetType& AssetObject) const
	{
		return Id == AssetObject.RuntimeId && UnderlyingTypeId = AssetTypeIdGetter<AssetType>::Value;
	}

	bool IsNull() const
	{
		return Id == AssetIdNull || UnderlyingTypeId == AssetTypeIdNull;
	}
	
	bool IsValid() const;

	const AssetType& GetAssetRef() const;

private:
	template <DerivedFromAsset OtherAssetType>
	AssetHandle(const OtherAssetType& Asset)
		: Id(Asset.RuntimeId)
		  , UnderlyingTypeId(Asset.TypeId)
	{
		static_assert(std::is_base_of_v<OtherAssetType, AssetType>, "AssetType must be derived from OtherAssetType");
		InternalAddRef();
	}

	AssetHandle(const AssetType& Asset)
		: Id(Asset.RuntimeId)
		  , UnderlyingTypeId(Asset.TypeId)
	{
		InternalAddRef();
	}

	void InternalAddRef()
	{
		if(IsNull()|| !IsValid())
		{
			return;
		}
		
		if (GetAssetRef().AddRef() == 1)
		{
			RemoveUnloadRequest();
		}
	}

	void InternalRelease()
	{
		if(IsNull() || !IsValid())
		{
			return;
		}
		
		if (GetAssetRef().Release() == 0 && !GetAssetRef().HasLoadingRefs())
		{
			RequestAssetUnload();
		}
	}

	void RequestAssetUnload();
	void RemoveUnloadRequest();

private:
	AssetId Id;
	AssetTypeId UnderlyingTypeId;
	friend class AssetManager;

	template<DerivedFromAsset OtherType>
	friend class AssetHandle;

	template<DerivedFromAsset OtherType>
	friend bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, AssetHandle<OtherType>& Value);

	template<DerivedFromAsset OtherType>
	friend bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const AssetHandle<OtherType>& Value);
};

class AssetManager : public ServiceBase
{
	using IdType = AssetId;
	

public:
	AssetManager() = default;

	void Initialize() override
	{
	}

	void Shutdown() override
	{
		for (auto& assetStorageIt : AssetStorages)
		{
			assetStorageIt.second->Clear();
		}
	}

	using size_type = std::size_t;


	AssetHandle<> GetAssetUsingPath(const Path& AssetPath)
	{
		Uid assetUid = GetServiceRegistry().GetService<AssetRegistry>().GetUidFromPath(AssetPath);
		if (!assetUid.IsValid())
		{
			LE_INFO("There is no Asset at the path: {}", AssetPath.generic_string());
			return AssetHandle<>{};
		}

		return GetAsset(assetUid);
	}
	
	template <DerivedFromAsset AssetType>
	AssetHandle<AssetType> GetAssetUsingPath(const Path& AssetPath)
	{
		return GetAssetUsingPath(AssetPath);
	}

	AssetHandle<> GetAsset(Uid AssetUid)
	{
		AssetRegistry& assetRegistry = GetServiceRegistry().GetService<AssetRegistry>();
		AssetInfo assetInfo = assetRegistry.GetAssetInfo(AssetUid);
		if (!assetInfo.IsValid())
		{
			LE_INFO("There is no Asset with UID: {}", Uid::ToString(assetInfo.AssetUid));
			return AssetHandle{};
		}

		if (HasAsset(assetInfo.RuntimeId, assetInfo.TypeId))
		{
			return AssetHandle{GetAssetRef(assetInfo.RuntimeId, assetInfo.TypeId)};
		}

		AssetStorageBase<IdType>& assetStorage = GetCreateAssetStorage(assetInfo.TypeId);
		Asset& newAsset = assetStorage.CreateAsset(assetInfo.AssetUid);
		assetRegistry.UpdateAssetRuntimeId(assetInfo.AssetUid, newAsset.GetAssetId());

		return AssetHandle{newAsset};
	}

	template <DerivedFromAsset AssetType>
	AssetHandle<AssetType> GetAsset(Uid AssetUid)
	{
		return GetAsset(AssetUid);
	}

	AssetHandle<Asset> GetAsset(AssetTypeId TypeId, Uid AssetUid)
	{
		AssetRegistry& assetRegistry = GetServiceRegistry().GetService<AssetRegistry>();
		AssetInfo assetInfo = assetRegistry.GetAssetInfo(AssetUid);
		if (!assetInfo.IsValid())
		{
			LE_INFO("There is no Asset with UID: {}", Uid::ToString(assetInfo.AssetUid));
			return AssetHandle<Asset>{};
		}

		if (HasAsset<Asset>(assetInfo.RuntimeId))
		{
			return AssetHandle<Asset>{GetAssetRef<Asset>(assetInfo.RuntimeId)};
		}
	}

	template <DerivedFromAsset AssetType>
	void LoadAsset(AssetHandle<AssetType> Handle) const
	{
		if (!Handle.IsValid() || Handle->IsLoaded())
		{
			return;
		}

		AssetDependencyLoaderResolver loaderResolver;
		loaderResolver.LoadAsset(Handle->GetStableId());
	}

	template <DerivedFromAsset AssetType>
	void LoadAssetAsync(AssetHandle<AssetType> Handle) const
	{
		if (!Handle.IsValid() || Handle->IsLoaded())
		{
			return;
		}

		AssetDependencyLoaderResolver loaderResolver;
		loaderResolver.LoadAssetAsync(Handle->GetStableId());
	}

	void OnFrameEnd();

private:
	void InternalLoadAsset(AssetInfo& Info) const;
	
	Asset& GetAssetRef(IdType Id, AssetTypeId TypeId) const
	{
		const AssetStorageBase<IdType>& assetStorage = *GetAssetStorage(TypeId);
		return assetStorage.GetResource(Id);
	}

	template <DerivedFromAsset AssetType>
	AssetType& GetAssetRef(IdType Id, AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value) const
	{
		return static_cast<AssetType&>(GetAssetRef(Id, TypeId));
	}

	void DequeueAssetUnload(IdType Id, AssetTypeId TypeId)
	{
		std::lock_guard lock(PendingDeleteMutex);
		PendingDelete[TypeId].erase(Id);
	}

	template <DerivedFromAsset AssetType>
	void DequeueAssetUnload(IdType Id, AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value)
	{
		DequeueAssetUnload(Id, TypeId);
	}

	void QueueAssetUnload(IdType Id, AssetTypeId TypeId)
	{
		std::lock_guard lock(PendingDeleteMutex);
		PendingDelete[TypeId].emplace(Id);
	}

	template <DerivedFromAsset AssetType>
	void QueueAssetUnload(IdType Id, AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value)
	{
		QueueAssetUnload(Id, TypeId);
	}

	bool HasAsset(IdType Id, AssetTypeId TypeId) const
	{
		const AssetStorageBase<IdType>* assetStorage = GetAssetStorage(TypeId);
		if (!assetStorage)
		{
			return false;
		}

		return assetStorage->Has(Id);
	}

	template <DerivedFromAsset AssetType>
	bool HasAsset(IdType Id, AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value) const
	{
		return HasAsset(Id, TypeId);
	}

	AssetStorageBase<IdType>& GetCreateAssetStorage(AssetTypeId TypeId)
	{
		auto it = AssetStorages.find(TypeId);
		if (it != AssetStorages.end())
		{
			return *it->second;
		}

		AssetStorageFactory& factory = GetServiceRegistry().GetService<AssetStorageFactory>();
		AssetStorageFactory::ReturnType storage = factory.Create(TypeId);
		AssetStorages.emplace(TypeId, storage);

		return *storage;
	}

	template <DerivedFromAsset AssetType>
	AssetStorage<IdType, AssetType>& GetCreateAssetStorage(AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value)
	{
		static_assert(!std::is_same_v<AssetType, IdType>, "Attempting to pass Asset Id as Asset Type Id");
		using AssetStorageType = AssetStorage<IdType, AssetType>;

		return static_cast<AssetStorageType&>(GetCreateAssetStorage(TypeId));
	}

	const AssetStorageBase<IdType>* GetAssetStorage(AssetTypeId TypeId) const
	{
		auto it = AssetStorages.find(TypeId);
		if (it != AssetStorages.end())
		{
			return it->second.get();
		}

		return nullptr;
	}

	template <DerivedFromAsset AssetType>
	const AssetStorage<IdType, AssetType>* GetAssetStorage(AssetTypeId TypeId = AssetTypeIdGetter<AssetType>::Value) const
	{
		static_assert(!std::is_same_v<AssetType, IdType>, "Attempting to pass Asset Id as Asset Type Id");
		using AssetStorageType = AssetStorage<IdType, AssetType>;

		return static_cast<const AssetStorageType*>(GetAssetStorage(TypeId));
	}

private:
	std::unordered_map<AssetTypeId, std::shared_ptr<AssetStorageBase<IdType>>> AssetStorages;
	std::unordered_map<AssetTypeId, std::unordered_set<AssetId>> PendingDelete;
	std::mutex PendingDeleteMutex;

	template <DerivedFromAsset AssetType>
	friend class AssetHandle;

	friend class AssetDependencyLoaderResolver;
};

REGISTER_SERVICE_TYPE(AssetManager, "AssetManager")

template <DerivedFromAsset AssetType>
bool AssetHandle<AssetType>::IsValid() const
{
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	return manager.HasAsset(Id, UnderlyingTypeId);
}

template <DerivedFromAsset AssetType>
const AssetType& AssetHandle<AssetType>::GetAssetRef() const
{
	const AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	return static_cast<const AssetType&>(manager.GetAssetRef(Id, UnderlyingTypeId));
}

template <DerivedFromAsset AssetType>
void AssetHandle<AssetType>::RequestAssetUnload()
{
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	manager.QueueAssetUnload(Id, UnderlyingTypeId);
}

template <DerivedFromAsset AssetType>
void AssetHandle<AssetType>::RemoveUnloadRequest()
{
	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	manager.DequeueAssetUnload(Id, UnderlyingTypeId);
}

template <DerivedFromAsset AssetType>
bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const AssetHandle<AssetType>& Value)
{
	using namespace LE::Archive;

	if (Value.IsNull() || !Value.IsValid())
	{
		return Serialize(Ctx, Writer, EmptyUid);
	}

	const Uid refAssetUid = Value->GetStableId();
	if (!refAssetUid.IsValid())
	{
		LE_ASSERT_DESC(false, "Invalid Asset Handle")
		return false;
	}

	if (!Serialize(Ctx, Writer, refAssetUid))
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
	using namespace LE::Archive;
	
	Uid refAssetUid;
	if (!Deserialize(Ctx, Reader, refAssetUid))
	{
		Ctx.Error.Desc = "Failed when reading UID for the referenced Asset";
		return false;
	}

	if (!refAssetUid.IsValid())
	{
		return true;
	}

	AssetManager& manager = GetServiceRegistry().GetService<AssetManager>();
	Value = manager.GetAsset<AssetType>(refAssetUid);
	if(!Value.IsValid())
	{
		LE_ERROR("Failed to load asset refference");
		return false;
	}
	
	return true;
}
}
