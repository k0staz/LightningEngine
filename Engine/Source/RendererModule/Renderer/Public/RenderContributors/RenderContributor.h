#pragma once
#include "RenderContributerCore.h"
#include "RHIResources.h"
#include "ECS/EcsEntity.h"
#include "Templates/NonCopyable.h"

namespace LE::Renderer
{
/**
 * @class RenderContributor
 * @brief Base class responsible for contributing rendering-related data within the rendering framework.
 *
 * This class serves as an abstract base for objects that contribute rendering data for a specific frame.
 * It provides interfaces for managing proxies associated with rendering entities, handling dynamic resource updates, and determining whether the contributor should be active during a given frame.
 */
class RenderContributor : public NonCopyable
{
public:
	RenderContributor() = default;
	RenderContributor(RenderContributorId InInstanceId, RenderContributorTypeId InTypeId)
		: InstanceId(InInstanceId),
		  TypeId(InTypeId)
	{}

	virtual ~RenderContributor() = default;

	RenderContributor(RenderContributor&&) = default;
	RenderContributor& operator=(RenderContributor&&) = default;

	/**
	 * @brief Retrieves the GPU virtual address for the rendering contributor's data for the current frame.
	 *
	 * @return The 64-bit GPU virtual address associated with the contributor's frame-specific data.
	 */
	uint64 GetThisFrameDataGPUAddress() const { return ThisFrameDataGpuAddress; }

	/**
	 * @brief Writes frame-specific dynamic resource data to the provided frame buffer.
	 *
	 * This method is responsible for populating the specified linear buffer with
	 * dynamic resource data that is relevant for the current frame's rendering operations.
	 * It serves as a customizable extension point for contributors to provide frame-dependent
	 * data to the rendering pipeline.
	 *
	 * @param FrameBuffer A reference-counted pointer to an instance of RHILinearBuffer
	 *                    that serves as the destination for the dynamic data writes.
	 */
	virtual void WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer) = 0;

	virtual void AddProxyToThisFrameContribution(EcsEntity Entity)
	{}

	virtual bool HasProxy(EcsEntity Entity) { return false; }

	virtual void AddProxy(EcsEntity Entity) {}

	virtual void RemoveProxy(EcsEntity Entity) {}

	virtual uint32 GetIndexCount() const { return 0; }

	virtual bool IsReady() const { return true; }

	RenderContributorId GetInstanceId() const
	{
		return InstanceId;
	}

	RenderContributorTypeId GetTypeId() const
	{
		return TypeId;
	}

	[[nodiscard]] virtual RenderContributorMetaType GetMetaType() const = 0;

	template <DerivedFromRenderContributor Type>
	bool IsOfType() const
	{
		return TypeId == RenderContributorTypeIdGetter<Type>::Value;
	}

	template <DerivedFromRenderContributor Type>
	Type& As()
	{
		return static_cast<Type&>(*this);
	}

protected:
	uint64 ThisFrameDataGpuAddress = 0;

private:
	RenderContributorId InstanceId = RenderContributorIdNull;
	RenderContributorTypeId TypeId = RenderContributorTypeIdNull;
};
}
