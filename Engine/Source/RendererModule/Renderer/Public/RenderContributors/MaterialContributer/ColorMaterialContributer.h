#pragma once

#include "Math/LinearColor.h"
#include "RenderContributors/RenderContributor.h"
#include "RenderResourceManager/RenderResourceManager.h"

namespace LE::Renderer
{
class ColorMaterialContributor : public RenderContributor
{
public:
    ColorMaterialContributor() = default;
    ColorMaterialContributor(RenderContributorId InInstanceId, RenderContributorTypeId InTypeId)
        : RenderContributor(InInstanceId, InTypeId)
    {}

    void WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer) override;
    [[nodiscard]] RenderContributorMetaType GetMetaType() const override { return RenderContributorMetaType::MaterialData; };

    struct MaterialFrameData
    {
        LinearColor Color;
    };

    void SetColor(LinearColor Color)
    {
        this->Color = Color;
    }

private:
    bool AreResourcesReady() const;

private:
    LinearColor Color;
};

REGISTER_RENDER_CONTRIBUTOR_TYPE(LE::Renderer::ColorMaterialContributor, "ColorMaterial", "IMaterialInterface",
                                 "Materials/ColorMaterial/ColorMaterial.slang")
}
