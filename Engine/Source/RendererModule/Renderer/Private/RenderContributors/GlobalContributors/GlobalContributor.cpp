#include "RenderContributors/GlobalContributors/GlobalContributor.h"

#include "Templates/Alignment.h"


namespace LE::Renderer
{

GlobalContributor::GlobalFrameDynamicData& GlobalContributor::GetDynamicData()
{
	return DynamicData;
}

void GlobalContributor::WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer)
{
	ThisFrameDataGpuAddress = FrameBuffer->GetCurrentGpuAddress();
	FrameBuffer->Write(&DynamicData, sizeof(GlobalFrameDynamicData));
}
}
