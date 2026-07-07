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

public:
    void AddProxy(EcsEntity Entity) override { ++ProxyUsersCount; }
    void RemoveProxy(EcsEntity Entity) override { --ProxyUsersCount; }
    [[nodiscard]] bool IsReady() const override { return AreResourcesReady();};

private:
    RenderResourceHandle<const TextureRenderResource> BaseColorTexture;
    uint32 ProxyUsersCount = 0;
};

REGISTER_RENDER_CONTRIBUTOR_TYPE(LE::Renderer::BaseMaterialContributor, "BaseMaterial", "IMaterialInterface",
                                 "Materials/BaseMaterial/BaseMaterial.slang")
}
