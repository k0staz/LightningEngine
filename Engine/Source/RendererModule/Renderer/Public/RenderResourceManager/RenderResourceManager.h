#pragma once
#include <semaphore>
#include <stack>

#include "RenderResourceRegistry.h"
#include "RenderResources.h"
#include "Assets/StaticMeshAsset.h"
#include "AssetManager/AssetManager.h"
#include "Assets/TextureAsset.h"
#include "Misc/Uid.h"
#include "Service/ServiceBase.h"

namespace LE::Renderer
{
template <DerivedFromRenderResource Resource>
class RenderResourceHandle
{
public:
    RenderResourceHandle()
        : ResourceId(NullId{}),
          ResourceTypeId(NullId{})
    {
    }

    RenderResourceHandle(const Resource& Object)
        : ResourceId(Object.Id),
          ResourceTypeId(Object.TypeId)
    {
        InternalAddRef();
    }

    RenderResourceHandle(const RenderResourceHandle& OtherHandle)
        : ResourceId(OtherHandle.ResourceId),
          ResourceTypeId(OtherHandle.ResourceTypeId)
    {
        InternalAddRef();
    }

    template <DerivedFromRenderResource OtherResource>
    RenderResourceHandle(const RenderResourceHandle<OtherResource>& OtherHandle)
        : ResourceId(OtherHandle.ResourceId),
          ResourceTypeId(OtherHandle.ResourceTypeId)
    {
        static_assert(std::is_base_of_v<OtherResource, Resource>, "Resource must be derived from OtherResource");
        InternalAddRef();
    }

    RenderResourceHandle& operator=(const RenderResourceHandle& OtherHandle)
    {
        if (ResourceId == OtherHandle.ResourceId && ResourceTypeId == OtherHandle.ResourceTypeId)
        {
            return *this;
        }

        InternalRelease();
        ResourceId = OtherHandle.ResourceId;
        ResourceTypeId = OtherHandle.ResourceTypeId;
        InternalAddRef();

        return *this;
    }

    template <DerivedFromRenderResource OtherResource>
    RenderResourceHandle& operator=(const RenderResourceHandle<OtherResource>& OtherHandle)
    {
        static_assert(std::is_base_of_v<OtherResource, Resource>, "Resource must be derived from OtherResource");
        if (ResourceId == OtherHandle.ResourceId && ResourceTypeId == OtherHandle.ResourceTypeId)
        {
            return *this;
        }

        InternalRelease();
        ResourceId = OtherHandle.ResourceId;
        ResourceTypeId = OtherHandle.ResourceTypeId;
        InternalAddRef();

        return *this;
    }

    RenderResourceHandle(RenderResourceHandle&& OtherHandle) noexcept
        : ResourceId(OtherHandle.ResourceId)
          , ResourceTypeId(OtherHandle.ResourceTypeId)
    {
        OtherHandle.ResourceId = NullId{};
        OtherHandle.ResourceTypeId = NullId{};
    }

    template <DerivedFromRenderResource OtherResource>
    RenderResourceHandle(RenderResourceHandle<OtherResource>&& OtherHandle) noexcept
        : ResourceId(OtherHandle.ResourceId)
          , ResourceTypeId(OtherHandle.ResourceTypeId)
    {
        static_assert(std::is_base_of_v<OtherResource, Resource>, "Resource must be derived from OtherResource");
        OtherHandle.ResourceId = NullId{};
        OtherHandle.ResourceTypeId = NullId{};
    }

    RenderResourceHandle& operator=(RenderResourceHandle&& OtherHandle) noexcept
    {
        if (ResourceId == OtherHandle.ResourceId && ResourceTypeId == OtherHandle.ResourceTypeId)
        {
            return *this;
        }

        InternalRelease();
        ResourceId = OtherHandle.ResourceId;
        ResourceTypeId = OtherHandle.ResourceTypeId;
        InternalAddRef();

        return *this;
    }

    template <DerivedFromRenderResource OtherResource>
    RenderResourceHandle& operator=(RenderResourceHandle<OtherResource>&& OtherHandle) noexcept
    {
        static_assert(std::is_base_of_v<OtherResource, Resource>, "Resource must be derived from OtherResource");
        if (ResourceId == OtherHandle.ResourceId && ResourceTypeId == OtherHandle.ResourceTypeId)
        {
            return *this;
        }

        InternalRelease();
        ResourceId = OtherHandle.ResourceId;
        ResourceTypeId = OtherHandle.ResourceTypeId;
        InternalAddRef();

        return *this;
    }

    ~RenderResourceHandle() noexcept
    {
        InternalRelease();
    }

    bool operator==(const RenderResourceHandle& Other) const
    {
        return ResourceId == Other.ResourceId && ResourceTypeId != Other.ResourceTypeId;
    }

    bool operator==(const Resource& ResourceObject) const
    {
        return ResourceId == ResourceObject.ResourceId && ResourceTypeId != ResourceObject.ResourceTypeId;
    }

    bool IsNull() const
    {
        return ResourceId == NullId{} && ResourceTypeId == NullId{};
    }

    RenderResourceId GetResourceId() const
    {
        return ResourceId;
    }

    RenderResourceTypeId GetResourceTypeId() const
    {
        return ResourceTypeId;
    }

    bool IsValid() const;
    Resource& Get() const;

    template <DerivedFromRenderResource OtherResource>
    OtherResource& GetAs() const
    {
        static_assert(std::is_base_of_v<Resource, OtherResource>, "OtherResource must be derived from Resource");
        return static_cast<OtherResource&>(Get());
    }

private:
    void InternalAddRef() const
    {
        if (!IsValid())
        {
            return;
        }

        Get().AddRef();
    }

    void ScheduleRelease() const;

    void InternalRelease() const
    {
        if (!IsValid())
        {
            return;
        }

        if (Get().ReleaseRef() == 0)
        {
            ScheduleRelease();
        }
    }

private:
    RenderResourceId ResourceId = NullId{};
    RenderResourceTypeId ResourceTypeId = NullId{};
};

/**
 * @brief Manages static global buffers for meshes and materials. Those are immutable between frames
 */
class RenderResourceManager : public ServiceBase
{
    struct PendingStaticMeshLoad;
    struct PendingTextureLoad;
    struct BatchTransferContext;

public:
    void Initialize() override;
    void Shutdown() override;
    void OnBeginFrame();

    void EnqueuePendingBarriers(RefCountingPtr<RHI::RHICommandList> CommandList);

    void DispatchBatchLoadTasks();
    void FinalizeBatchLoad();

    RenderResourceHandle<const StaticMeshRenderResource> RequestStaticMesh(AssetHandle<StaticMeshAsset> Asset);
    RenderResourceHandle<const TextureRenderResource> RequestTexture(AssetHandle<TextureAsset> Asset);

    bool HasResource(RenderResourceTypeId TypeId, RenderResourceId ResourceId) const;

    RefCountingPtr<RHI::RHIDescriptorSetLayout> GetGlobalDescriptorSetLayout() const;
    RefCountingPtr<RHI::RHIDescriptorSet> GetGlobalDescriptorSet() const;
private:
    void RecordTransferCommandsStaticMeshTask(std::vector<PendingStaticMeshLoad> PendingStaticMeshLoad);
    void RecordTransferCommandsTextureTask(std::vector<PendingTextureLoad> PendingTextureLoad);
    void OnTransferTaskEnd(RefCountingPtr<RHI::RHICommandList> CommandList);
    RenderResource& GetRenderResource(RenderResourceTypeId TypeId, RenderResourceId ResourceId) const;

    void RecordPendingResourceDelete(RenderResourceTypeId TypeId, RenderResourceId ResourceId);
    void ReleaseResource(RenderResourceTypeId TypeId, RenderResourceId ResourceId);
    void ReleaseStaticMesh(RenderResourceId ResourceId);
    void ReleaseTexture(RenderResourceId ResourceId);
    void OnBeginFrameProcessPendingDelete();

    uint64 GetFreeTextureBindingSlot();

    template <DerivedFromRenderResource Resource>
    RenderResourceStorageBase& GetCreateResourceStorage(RenderResourceTypeId TypeId = RenderResourceTypeIdGetter<Resource>::Value)
    {
        auto it = ResourceStorages.find(TypeId);
        if (it != ResourceStorages.end())
        {
            return *it->second;
        }

        ResourceStorages[TypeId] = std::make_shared<RenderResourceStorage<Resource>>();
        return *ResourceStorages[TypeId];
    }

    RenderResourceStorageBase* GetResourceStorage(RenderResourceTypeId TypeId) const;

private:
    struct PendingStaticMeshLoad
    {
        RenderResourceHandle<StaticMeshRenderResource> ResourceHandle;
        AssetHandle<StaticMeshAsset> Asset;
    };

    struct PendingTextureLoad
    {
        RenderResourceHandle<TextureRenderResource> ResourceHandle;
        AssetHandle<TextureAsset> Asset;
    };

    struct PendingResourceDelete
    {
        RenderResourceTypeId TypeId;
        RenderResourceId ResourceId;
    };

    struct PendingResourceBarriers
    {
        uint32 TimelineValue = 0;
        std::vector<RHI::RHIImageMemoryBarrierDesc> ImageBarriers;
    };

    struct BatchTransferContext
    {
        std::mutex ContextMutex;
        std::vector<RefCountingPtr<RHI::RHICommandList>> FinalizedList;
        PendingResourceBarriers PendingBarriers;
        uint32 RemainingTaskCount;
        bool WasStarted;
        std::binary_semaphore CompletionSignal{0};
    };

    struct StaticMeshChannels
    {
        RefCountingPtr<RHI::RHIGlobalBufferChannel> IndexChannel = nullptr;
        RefCountingPtr<RHI::RHIGlobalBufferChannel> PositionChannel = nullptr;
        RefCountingPtr<RHI::RHIGlobalBufferChannel> NormalsChannel = nullptr;
        RefCountingPtr<RHI::RHIGlobalBufferChannel> TexCoordChannel = nullptr;
    };

    struct FrameTransferContext
    {
        RefCountingPtr<RHI::RHILinearBuffer> StageBuffer = nullptr;
    };

    struct TextureSlots
    {
        std::mutex SlotMutex;
        uint64 HeadSlotIndex = RHI::TRANSIENT_TEXTURE_SLOT_COUNT;
        std::stack<uint64> FreeSlots;
    };

    std::unordered_map<RenderResourceTypeId, std::shared_ptr<RenderResourceStorageBase>> ResourceStorages;

    RefCountingPtr<RHI::RHIGlobalBuffer> GlobalMeshBuffer;
    StaticMeshChannels StaticMeshChannels;

    std::mutex StaticMeshLoadsMutex;
    std::vector<PendingStaticMeshLoad> PendingStaticMeshLoads;

    std::mutex TextureLoadsMutex;
    std::vector<PendingTextureLoad> PendingTextureLoads;

    std::mutex PendingResourceDeletesMutex;
    std::array<std::vector<PendingResourceDelete>, DEFAULT_FRAMES_IN_FLIGHT> PendingResourceDeletes;

    std::array<RefCountingPtr<RHI::RHISampler>, static_cast<std::size_t>(RHI::RHISamplerType::Count)> GlobalSamplers;
    RefCountingPtr<RHI::RHIDescriptorSetPool> GlobalDescriptorSetPool;
    RefCountingPtr<RHI::RHIDescriptorSetLayout> GlobalDescriptorSetLayout;
    RefCountingPtr<RHI::RHIDescriptorSet> GlobalDescriptorSet;
    TextureSlots TextureBindingSlots;

    BatchTransferContext ThisFrameBatchContext = {};
    FrameTransferContext FrameTransferContexts[DEFAULT_FRAMES_IN_FLIGHT][DEFAULT_TASK_WORKER_THREADS];

    std::stack<PendingResourceBarriers> PendingBarriers;
    std::mutex PendingBarriersMutex;

    template <DerivedFromRenderResource Resource>
    friend class RenderResourceHandle;
};

template <DerivedFromRenderResource Resource>
bool RenderResourceHandle<Resource>::IsValid() const
{
    if (IsNull())
    {
        return false;
    }

    auto& manager = GetServiceRegistry().GetService<RenderResourceManager>();
    return manager.HasResource(ResourceTypeId, ResourceId);
}

template <DerivedFromRenderResource Resource>
Resource& RenderResourceHandle<Resource>::Get() const
{
    auto& manager = GetServiceRegistry().GetService<RenderResourceManager>();
    return static_cast<Resource&>(manager.GetRenderResource(ResourceTypeId, ResourceId));
}

template <DerivedFromRenderResource Resource>
void RenderResourceHandle<Resource>::ScheduleRelease() const
{
    auto& manager = GetServiceRegistry().GetService<RenderResourceManager>();
    manager.RecordPendingResourceDelete(ResourceTypeId, ResourceId);
}
}

namespace LE
{
REGISTER_SERVICE_TYPE(Renderer::RenderResourceManager, "RenderResourceManager")
}
