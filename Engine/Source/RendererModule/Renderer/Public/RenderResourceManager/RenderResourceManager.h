#pragma once
#include <semaphore>

#include "RenderResourceRegistry.h"
#include "RenderResources.h"
#include "Assets/StaticMeshAsset.h"
#include "AssetManager/AssetManager.h"
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

    void InternalRelease() const
    {
        if (!IsValid())
        {
            return;
        }

        Get().ReleaseRef();
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
    struct BatchTransferContext;

public:
    void Initialize() override;
    void Shutdown() override;
    void OnBeginFrame();

    void DispatchBatchLoadTasks();
    void FinalizeBatchLoad();

    RenderResourceHandle<const StaticMeshRenderResource> RequestStaticMesh(AssetHandle<StaticMeshAsset> Asset);
    RenderContributorId GetRenderContributor(RenderResourceHandle<const StaticMeshRenderResource> StaticMeshResource) const;

    bool HasResource(RenderResourceId ResourceId) const;

    void ReleaseStaticMesh(RenderResourceHandle<const StaticMeshRenderResource> StaticMeshResource);
    void ReleaseStaticMesh(RenderResourceId ResourceId);

private:
    void RecordTransferCommandsTask(std::vector<PendingStaticMeshLoad> PendingStaticMeshLoad);
    RenderResource& GetRenderResource(RenderResourceId ResourceId) const;

private:
    struct PendingStaticMeshLoad
    {
        RenderResourceHandle<StaticMeshRenderResource> ResourceHandle;
        AssetHandle<StaticMeshAsset> Asset;
    };

    struct BatchTransferContext
    {
        std::mutex ContextMutex;
        std::vector<RefCountingPtr<RHI::RHICommandList>> FinalizedList;
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

    StaticMeshRenderResourceStorage StaticMeshStorage;
    RefCountingPtr<RHI::RHIGlobalBuffer> GlobalMeshBuffer;
    StaticMeshChannels StaticMeshChannels;

    std::mutex StaticMeshLoadsMutex;
    std::vector<PendingStaticMeshLoad> PendingStaticMeshLoads;

    BatchTransferContext ThisFrameBatchContext = {};
    FrameTransferContext FrameTransferContexts[DEFAULT_FRAMES_IN_FLIGHT][DEFAULT_TASK_WORKER_THREADS];

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

    RenderResourceManager& manager = GetServiceRegistry().GetService<RenderResourceManager>();
    return manager.HasResource(ResourceId);
}

template <DerivedFromRenderResource Resource>
Resource& RenderResourceHandle<Resource>::Get() const
{
    RenderResourceManager& manager = GetServiceRegistry().GetService<RenderResourceManager>();
    return static_cast<Resource&>(manager.GetRenderResource(ResourceId));
}
}

namespace LE
{
REGISTER_SERVICE_TYPE(Renderer::RenderResourceManager, "RenderResourceManager")
}
