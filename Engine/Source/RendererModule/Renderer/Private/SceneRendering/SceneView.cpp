#include "SceneRendering/SceneView.h"

#include "Math/MathUtils.h"

namespace LE::Renderer
{
SceneView::SceneView(const SceneViewInfo& InitInfo)
    : ViewInfo(InitInfo)
{
}

void SceneView::SetupViewMatrices()
{
    ThisFrameViewMatrices.WorldToView = Matrix4x4F::GetInverted(ViewInfo.ViewTransform);

    if (!ViewInfo.RhiWindow.IsValid())
    {
        return;
    }

    const RHI::RHIWindow& RhiWindow = *ViewInfo.RhiWindow;

    if (RhiWindow.GetHeight() == 0)
    {
        LE_ASSERT(false)
        return;
    }

    const float aspectRatio = static_cast<float>(RhiWindow.GetWidth()) / static_cast<float>(RhiWindow.GetHeight());
    const float radiansFov = MathUtils::DegreesToRadians(ViewInfo.FOV);
    const float g = 1.0f / Tan(radiansFov * 0.5f);

    static constexpr float floatErrorCorrection = 1e-5f;
    ThisFrameViewMatrices.ViewToClip = Matrix4x4F(g / aspectRatio, 0.0f, 0.0f, 0.0f,
                                                  0.0f, g, 0.0f, 0.0f,
                                                  0.0f, 0.0f, 1.0f - floatErrorCorrection, 1.0f,
                                                  0.0f, 0.0f, -ViewInfo.NearPlane * (1.0f - floatErrorCorrection), 0.0f);
}
}
