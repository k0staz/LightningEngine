#include "RenderPasses/BaseRenderPass.h"

namespace LE::Renderer
{
	REGISTER_MATERIAL_PASS_VARIANT(BaseVS, "testVertexShader.hlsl", "VSMain" ,RHI::ShaderType::Vertex, "BaseMaterial", RenderPassType::Base)
	REGISTER_MATERIAL_PASS_VARIANT(BasePS, "testPixelShader.hlsl", "PSMain" ,RHI::ShaderType::Pixel, "BaseMaterial", RenderPassType::Base)
}