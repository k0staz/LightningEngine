#include "RHIResources.h"


namespace LE::RHI
{

bool RHIBuffer::IsGlobalBuffer() const
{
	switch (Description.UsageType)
	{
	case RHIBufferUsageType::MeshGlobal:
	case RHIBufferUsageType::MaterialGlobal:
		return true;
	default:
		return false;
	}
}

bool RHIBufferSubAllocation::IsSubAllocatedFrom(RHIBufferUsageType GlobalBufferUsageType) const
{
	return OwnerBufferUsageType == GlobalBufferUsageType;
}
}
