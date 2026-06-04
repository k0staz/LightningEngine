#pragma once
#include "CoreMinimum.h"

#include "ShaderParameterTypeDescriptors.h"
#include "Math/Matrix4x4.h"

namespace LE::Renderer
{
class Viewport;
BEGIN_GLOBAL_CONSTANT_BUFFER(ViewShaderParametersConstantBuffer)
	DECLARE_SHADER_PARAMETER(Matrix4x4F, WorldToView)
	DECLARE_SHADER_PARAMETER(Matrix4x4F, ViewToClip)
END_CONSTANT_BUFFER()

struct SceneViewMatrices
{
	SceneViewMatrices()
		: WorldToView(Matrix4x4F::Identity())
		  , ViewToClip(Matrix4x4F::Identity())
	{
	}

	Matrix4x4F WorldToView;
	Matrix4x4F ViewToClip;
};

struct SceneViewInfo
{
	SceneViewInfo()
		: ViewTransform(Matrix4x4F::Identity())
		  , FOV(90.f)
		  , NearPlane(0.2f)
	{
	}

	Matrix4x4F ViewTransform;
	float FOV;
	float NearPlane;
};

class SceneView
{
public:
	SceneView(const SceneViewInfo& InitInfo);

	void SetupViewMatrices(Viewport* Viewport);

	void InitResourcesRHI();
	void CreateConstantBuffer(const ViewShaderParametersConstantBuffer& Parameters);

	ConstantBufferRef<ViewShaderParametersConstantBuffer> ConstantBuffer;
	SceneViewMatrices ViewMatrices;
	Matrix4x4F ViewTransform;
	float FOV;
	float NearPlane;

protected:
	friend class SceneRender;
};
}
