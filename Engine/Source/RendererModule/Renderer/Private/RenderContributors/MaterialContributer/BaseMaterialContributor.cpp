#include "RenderContributors/MaterialContributer/BaseMaterialContributor.h"

namespace LE::Renderer
{
void BaseMaterialContributor::WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer)
{
    if (!AreResourcesReady())
    {
        return;
    }

    ThisFrameDataGpuAddress = FrameBuffer->GetCurrentGpuAddress();
    MaterialFrameData frameData = {};
    frameData.BaseColorTextureIndex = BaseColorTexture.Get().BindingSlot;
    frameData.BaseColorSamplerIndex = static_cast<uint32>(RHI::RHISamplerType::LinearRepeat);
    FrameBuffer->Write(&frameData, sizeof(MaterialFrameData));
}

bool BaseMaterialContributor::AreResourcesReady() const
{
    if (!BaseColorTexture.IsValid())
    {
        return false;
    }

    return BaseColorTexture.Get().IsLoaded();
}
}
