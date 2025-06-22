#pragma once
#include "ShaderParameterTypeDescriptors.h"
#include "Materials/Material.h"

namespace LE::Renderer
{
class BaseVS : Shader
{
	DECLARE_MATERIAL_PASS_VARIANT(BaseVS)
	BEGIN_ROOT_CONSTANT_BUFFER(RootConstantBuffer, BaseVS)
	END_CONSTANT_BUFFER()
};

class BasePS : Shader
{
	DECLARE_MATERIAL_PASS_VARIANT(BasePS)

	BEGIN_MATERIAL_CONSTANT_BUFFER(BasePSConstantBuffer)
		DECLARE_SHADER_PARAMETER(Vector4F, Color)
	END_CONSTANT_BUFFER()

	BEGIN_ROOT_CONSTANT_BUFFER(RootConstantBuffer, BasePS)
		INCLUDE_CONSTANT_BUFFER(BasePSConstantBuffer, MaterialParameters)
	END_CONSTANT_BUFFER()

};
}
