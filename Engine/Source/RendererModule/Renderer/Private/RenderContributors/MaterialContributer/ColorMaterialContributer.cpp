#include "RenderContributors/MaterialContributer/ColorMaterialContributer.h"

namespace LE::Renderer
{
void ColorMaterialContributor::WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer)
{
    ThisFrameDataGpuAddress = FrameBuffer->GetCurrentGpuAddress();
    MaterialFrameData frameData = {};
    frameData.Color = Color;
    FrameBuffer->Write(&frameData, sizeof(MaterialFrameData));
}

bool ColorMaterialContributor::AreResourcesReady() const
{
    return true;
}
}
