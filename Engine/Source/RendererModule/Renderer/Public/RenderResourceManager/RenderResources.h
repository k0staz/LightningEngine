#pragma once
#include "Templates/RefCounters.h"
#include "RHIResources.h"

namespace LE::Renderer
{
using RenderResourceId = uint32;
using RenderResourceTypeId = uint32;

template <typename Type>
struct RenderResourceTraits : IdTraitsInterpreter<IdTraits<Type>>
{
	static constexpr uint64 PageSize = 4096;
};

enum class RenderResourceState
{
	Unloaded = 0,
	Loading,
	Ready,
};

template <typename Type>
concept DerivedFromRenderResource = std::is_base_of_v<class RenderResource, Type>;

/**
 * @brief Base abstraction for all manager-tracked graphics resources.
 *
 * This class serves as the foundation for resources managed within a sparse set layout.
 * It encapsulates a unique tracking identifier and provides thread-safe access to 
 * the asset's current loading and lifecycle state.
 */
class RenderResource : public NonCopyable
{
public:
	RenderResource() = default;

	RenderResource(RenderResourceId Id, RenderResourceTypeId TypeId)
		: Id(Id)
		, TypeId(TypeId)
	{}
	
	RenderResource(RenderResource&& Other) noexcept
		: Id(Other.Id)
		, TypeId(Other.TypeId)
		, State(Other.State.load(std::memory_order_acquire))
		, RefsNum(Other.RefsNum.load(std::memory_order_acquire))
		, BatchTransferValue(Other.BatchTransferValue)
	{}

	RenderResource& operator=(RenderResource&& Other) noexcept
	{
		if (this != &Other)
		{
			using std::swap;
			swap(Id, Other.Id);
			swap(TypeId, Other.TypeId);

			const RenderResourceState state = State.load(std::memory_order_acquire);
			const RenderResourceState otherState = Other.State.load(std::memory_order_acquire);
			State.store(otherState, std::memory_order_release);
			Other.State.store(state, std::memory_order_release);

			const auto refNum = RefsNum.load(std::memory_order_acquire);
			const auto otherRefNum = Other.RefsNum.load(std::memory_order_acquire);
			RefsNum.store(otherRefNum, std::memory_order_release);
			Other.RefsNum.store(refNum, std::memory_order_release);

			swap(BatchTransferValue, Other.BatchTransferValue);
		}

		return *this;
	}

	RenderResourceState GetState() const { return State.load(std::memory_order_acquire); }

	bool IsLoaded() const;
	uint64 GetBatchTransferValue() const { return BatchTransferValue; }
	
	RenderResourceId GetId() const { return Id; }
	RenderResourceTypeId GetTypeId() const { return TypeId; }

	uint32 GetRefsNum() const { return RefsNum.load(std::memory_order_acquire); }

private:
	uint32 AddRef() const;
	uint32 ReleaseRef() const;

	void SetState(RenderResourceState NewState) const;
	bool TrySetLoadingState();

	void SetBatchTransferValue(uint64 Value) { BatchTransferValue = Value; };

private:
	mutable std::atomic<RenderResourceState> State = RenderResourceState::Unloaded;
	RenderResourceId Id = NullId();
	RenderResourceTypeId TypeId = NullId();
	mutable std::atomic_uint RefsNum = 0;
	uint64 BatchTransferValue = UINT64_MAX;

	friend class RenderResourceManager;

	template<DerivedFromRenderResource ResourceType>
	friend class RenderResourceHandle;
};

template<DerivedFromRenderResource ResourceType>
struct RenderResourceTypeRegistration;

template<DerivedFromRenderResource ResourceType>
struct RenderResourceTypeIdGetter
{
	static constexpr std::string_view TypeName = RenderResourceTypeRegistration<ResourceType>::Value;
	static constexpr RenderResourceTypeId Value = FNV1AHash(TypeName);
};

#define REGISTER_RENDER_RESOURCE_TYPE(Type, TypeName) \
	template<> \
	struct RenderResourceTypeRegistration<Type> \
	{ \
		static constexpr std::string_view Value = TypeName; \
	};


class StaticMeshRenderResource : public RenderResource
{
public:
	using RenderResource::RenderResource;
	
	StaticMeshRenderResource(StaticMeshRenderResource&& other) noexcept = default;
	StaticMeshRenderResource& operator=(StaticMeshRenderResource&& other) noexcept = default;

	uint32 IndexCount = 0;
	RefCountingPtr<RHI::RHIBufferSubAllocation> IndexBuffer;
	RefCountingPtr<RHI::RHIBufferSubAllocation> PositionBuffer;
	RefCountingPtr<RHI::RHIBufferSubAllocation> NormalBuffer;
	RefCountingPtr<RHI::RHIBufferSubAllocation> TexCoordsBuffer;
};

REGISTER_RENDER_RESOURCE_TYPE(StaticMeshRenderResource, "StaticMeshRenderResource")

class TextureRenderResource : public RenderResource
{
public:
	using RenderResource::RenderResource;

	TextureRenderResource(TextureRenderResource&& other) noexcept = default;
	TextureRenderResource& operator=(TextureRenderResource&& other) noexcept = default;

	uint64 BindingSlot = 0;
	RefCountingPtr<RHI::RHIImage> Texture = nullptr;
	RefCountingPtr<RHI::RHIImageView> TextureView = nullptr;
};

REGISTER_RENDER_RESOURCE_TYPE(TextureRenderResource, "TextureRenderResource")
}
