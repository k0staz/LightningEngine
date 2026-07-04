#pragma once

#include "RenderContributors/RenderContributor.h"
#include "RenderResourceManager/RenderResourceManager.h"

namespace LE::Renderer
{
class BaseMaterialContributor : public RenderContributor
{
public:
    BaseMaterialContributor() = default;
    BaseMaterialContributor(RenderContributorId InInstanceId, RenderContributorTypeId InTypeId)
        : RenderContributor(InInstanceId, InTypeId)
    {}

    void WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer) override;
    [[nodiscard]] RenderContributorMetaType GetMetaType() const override { return RenderContributorMetaType::MaterialData; };

    struct MaterialFrameData
    {
        uint64 BaseColorTextureIndex;
        uint32 BaseColorSamplerIndex;
    };

    void SetBaseColorTexture(RenderResourceHandle<const TextureRenderResource> Texture)
    {
        BaseColorTexture = Texture;
    }

private:
    bool AreResourcesReady() const;

private:
    RenderResourceHandle<const TextureRenderResource> BaseColorTexture;
};

REGISTER_RENDER_CONTRIBUTOR_TYPE(LE::Renderer::BaseMaterialContributor, "BaseMaterial", "IMaterialInterface",
                                 "Materials/BaseMaterial/BaseMaterial.slang")
}
