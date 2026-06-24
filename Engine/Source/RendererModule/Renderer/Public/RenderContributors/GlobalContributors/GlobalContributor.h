#pragma once

#include "Math/Matrix4x4.h"
#include "RenderContributors/RenderContributor.h"
#include "RenderContributors/RenderContributorInstanceStorage.h"
#include "Templates/Alignment.h"

namespace LE::Renderer
{
class GlobalContributor : public RenderContributor
{
public:
	GlobalContributor() = default;
	GlobalContributor(RenderContributorId InInstanceId, RenderContributorTypeId InTypeId)
		: RenderContributor(InInstanceId, InTypeId)
	{}
	
	GlobalContributor(GlobalContributor&&) = default;
	GlobalContributor& operator=(GlobalContributor&&) = default;

	struct GlobalFrameDynamicData
	{
		Matrix4x4F WorldToView;
		Matrix4x4F ViewToClip;
	};

	GlobalFrameDynamicData& GetDynamicData();
	void WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer) override;

	RenderContributorMetaType GetMetaType() const override { return RenderContributorMetaType::GlobalData; }

private:
	GlobalFrameDynamicData DynamicData = {};
};

REGISTER_RENDER_CONTRIBUTOR_TYPE(LE::Renderer::GlobalContributor, "GlobalStandard", "IGlobal", "Core/GlobalBindless.slang")
}
