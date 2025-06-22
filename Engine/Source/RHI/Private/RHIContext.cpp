#include "RHIContext.h"

#include "DynamicRHI.h"

namespace LE::RHI
{
RefCountingPtr<RHIBoundShaderState> RHICreateBoundShaderState(RHIVertexShader* VertexShader, RHIPixelShader* PixelShader)
{
	return gDynamicRHI->RHICreateBoundShaderState(VertexShader, PixelShader);
}

void RHIContextNonNativePSO::RHISetPSO(RHIPipelineStateObject* GraphicsPSO, uint32 StencilRef)
{
	RHINonNativePipelineStateObject* nonNativePipeline = static_cast<RHINonNativePipelineStateObject*>(GraphicsPSO);
	SetPSOFromInitializer(nonNativePipeline->Initializer, StencilRef);
}

void RHIContextNonNativePSO::SetPSOFromInitializer(const PipelineStateInitializer& PSOInit, uint32 StencilRef)
{
	RHISetBoundShaderState(RHICreateBoundShaderState(PSOInit.ShaderState.VertexShaderRHI, PSOInit.ShaderState.PixelShaderRHI));
	RHISetDepthStencilState(PSOInit.DepthStencilState, StencilRef);
}
}
