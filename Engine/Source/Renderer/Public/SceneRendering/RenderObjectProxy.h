#pragma once
#include "MeshGroup.h"
#include "ShaderParameterTypeDescriptors.h"
#include "ECS/EcsEntity.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"


namespace LE::Renderer
{
BEGIN_GLOBAL_CONSTANT_BUFFER(ObjectShaderParameters)
	DECLARE_SHADER_PARAMETER(Matrix4x4F, LocalToWorld)
END_CONSTANT_BUFFER()

class RenderObjectProxy
{
public:
	virtual ~RenderObjectProxy() = default;
	RenderObjectProxy(EcsEntity OwnerEntity);

	EcsEntity GetOwnerEntity() const { return Owner; }
	const Matrix4x4F& GetTransform() const { return LocalToWorld; }

	void CreateConstantBuffer();
	void UpdateConstantBuffer(RenderCommandList& CmdList);

	RHI::RHIConstantBuffer* GetConstantBuffer() const
	{
		return ConstantBuffer.GetPointer();
	}

	void SetTransform(const Matrix4x4F& InLocalToWorld);

	virtual void GetMeshGroup(MeshGroup& OutMeshGroup) = 0;

protected:
	EcsEntity Owner;
	Matrix4x4F LocalToWorld;
	ConstantBufferRef<ObjectShaderParameters> ConstantBuffer;
};
}
