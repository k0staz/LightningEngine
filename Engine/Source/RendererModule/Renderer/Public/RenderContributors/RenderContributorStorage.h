#pragma once

#include "RenderContributerCore.h"
#include "RenderContributor.h"
#include "Containers/ResourceSparseSet.h"

namespace LE::Renderer
{
template<DerivedFromRenderContributor ContributorType>
class RenderContributorStorage : public ResourceSparseSet<RenderContributorId, ContributorType, RenderContributor, RenderContributorTraits<RenderContributorId>>
{
	using base_type = ResourceSparseSet<RenderContributorId, ContributorType, RenderContributor, RenderContributorTraits<RenderContributorId>>;
public:
	RenderContributor& CreateNewResource() override
	{
		RenderContributorId newResourceId = base_type::GetAvailableId();
		RenderContributorId& sparse = base_type::TryAdd(newResourceId);
		ContributorType* resource = std::to_address(base_type::GetCreateResource(base_type::GetIndex(sparse)));
		std::construct_at(resource, newResourceId, RenderContributorTypeIdGetter<ContributorType>::Value);
		return static_cast<RenderContributor&>(*resource);
	}
};
}