#include "SceneRendering/SceneView.h"

#include "Viewport.h"

namespace LE::Renderer
{
IMPLEMENT_GLOBAL_CONSTANT_BUFFER(ViewShaderParametersConstantBuffer, "ViewShaderParametersConstantBuffer")

SceneView::SceneView(const SceneViewInfo& InitInfo)
	: ViewTransform(InitInfo.ViewTransform)
	  , FOV(InitInfo.FOV)
	  , NearPlane(InitInfo.NearPlane)
{
}

void SceneView::SetupViewMatrices(Viewport* Viewport)
{
	ViewMatrices.WorldToView = Matrix4x4F::GetInverted(ViewTransform);

	if (Viewport->GetSizeY() == 0)
	{
		LE_ASSERT(false)
		return;
	}

	const float aspectRatio = static_cast<float>(Viewport->GetSizeX()) / static_cast<float>(Viewport->GetSizeY());
	const float g = 1.0f / Tan(FOV * 0.5f);

	static constexpr  float floatErrorCorrection = 1e-5f;
	ViewMatrices.ViewToClip = Matrix4x4F(g / aspectRatio, 0.0f, 0.0f, 0.0f,
	                                     0.0f, g, 0.0f, 0.0f,
	                                     0.0f, 0.0f, floatErrorCorrection, NearPlane * (1.0f - floatErrorCorrection),
	                                     0.0f, 0.0f, 1.0f, 0.0f);
}

void SceneView::InitResourcesRHI()
{
	ViewShaderParametersConstantBuffer parameters;
	parameters.ViewToClip = ViewMatrices.ViewToClip;
	parameters.WorldToView = ViewMatrices.WorldToView;
	CreateConstantBuffer(parameters);
}

void SceneView::CreateConstantBuffer(const ViewShaderParametersConstantBuffer& Parameters)
{
	ConstantBuffer = ConstantBufferRef<ViewShaderParametersConstantBuffer>::CreateConstantBuffer(Parameters);
}
}
