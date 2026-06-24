#pragma once
#include "CoreMinimum.h"
#include "RHIResources.h"

#include "Math/Matrix4x4.h"

namespace LE::Renderer
{
struct SceneViewInfo
{
	SceneViewInfo()
		: RhiWindow(nullptr)
		  , ViewTransform(Matrix4x4F::Identity())
		  , FOV(90.f)
		  , NearPlane(0.2f)
	{}

	RefCountingPtr<RHI::RHIWindow> RhiWindow;
	Matrix4x4F ViewTransform;
	float FOV;
	float NearPlane;
};

class SceneView
{
public:
	struct ViewMatrices
	{
		Matrix4x4F WorldToView;
		Matrix4x4F ViewToClip;
	};

	SceneView(const SceneViewInfo& InitInfo);

	void SetupViewMatrices();

	SceneViewInfo ViewInfo;
	ViewMatrices ThisFrameViewMatrices;

protected:
	friend class SceneRender;
};
}
