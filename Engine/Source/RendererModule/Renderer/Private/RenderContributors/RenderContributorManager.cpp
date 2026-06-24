#include "RenderContributors/RenderContributorManager.h"

#include <ranges>

namespace LE::Renderer
{
void RenderContributorManager::Initialize() {}

void RenderContributorManager::Shutdown()
{
	for (auto& val : ContributorStorages | std::views::values)
	{
		val->Clear();
	}

	ContributorStorages.clear();
}

void RenderContributorManager::WriteContributorsFrameData(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer) const
{
	for (const auto& val : ContributorStorages | std::views::values)
	{
		val->RunOnEach([&FrameBuffer](RenderContributor& Contributor)
		{
			Contributor.WriteFrameDataDynamicResources(FrameBuffer);
		});
	}
}
}
