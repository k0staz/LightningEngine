#include "SceneRendering/RenderObjectProxy.h"

namespace LE::Renderer
{
IMPLEMENT_GLOBAL_CONSTANT_BUFFER(ObjectShaderParameters, "ObjectShaderParameters")

RenderObjectProxy::RenderObjectProxy(EcsEntity OwnerEntity)
	: Owner(OwnerEntity)
	  , LocalToWorld(Matrix4x4F::Identity())
{
}

void RenderObjectProxy::CreateConstantBuffer()
{
	if (!ConstantBuffer)
	{
		ObjectShaderParameters parameters;
		parameters.LocalToWorld = LocalToWorld;
		parameters.LocalToWorld.Transpose();

		ConstantBuffer = ConstantBufferRef<ObjectShaderParameters>::CreateConstantBuffer(parameters);
	}
}

void RenderObjectProxy::UpdateConstantBuffer(RenderCommandList& CmdList)
{
	ObjectShaderParameters parameters;
	parameters.LocalToWorld = LocalToWorld;
	parameters.LocalToWorld.Transpose();
	CmdList.EnqueueLambdaCommand([parameters, this](Renderer::RenderCommandList& CmdList)
		{
			ConstantBuffer.UpdateConstantBuffer(CmdList, parameters);
		});


}

void RenderObjectProxy::SetTransform(const Matrix4x4F& InLocalToWorld)
{
	LocalToWorld = InLocalToWorld;
}
}
