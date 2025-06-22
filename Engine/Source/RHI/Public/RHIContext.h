#pragma once
#include "RHIResources.h"
#include "RHIShaderParameters.h"

namespace LE::RHI
{
class RHIContext
{
public:
	virtual void RHISetPSO(RHIPipelineStateObject* GraphicsPSO, uint32 StencilRef) = 0;
	virtual void RHISetShaderParameters(RHIShader* Shader, const Array<uint8>& ParametersData, const Array<RHIShaderParameter>& Parameters,
	                                    const Array<RHIShaderParameterResource>& ResourceParameters) = 0;

	virtual void RHISetShaderParameter(RHIShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 BytesSize, const void* Value) = 0;
	virtual void RHISetShaderTexture(RHIShader* Shader, uint32 TextureIndex, RHITexture* Texture) = 0;
	virtual void RHISetShaderReadViewParameter(RHIShader* Shader, uint32 SamplerIndex, RHIReadView* ReadView) = 0;
	virtual void RHISetShaderWriteViewParameter(RHIShader* Shader, uint32 WriteViewIndex, RHIWriteView* WriteView) = 0;
	virtual void RHISetShaderSampler(RHIShader* Shader, uint32 SamplerIndex, RHISamplerState* SamplerState) = 0;
	virtual void RHISetShaderConstantBuffer(RHIShader* Shader, uint32 BufferIndex, RHIConstantBuffer* ConstantBuffer) = 0;

	virtual void RHIDrawIndexedPrimitive(RHIBuffer* IndexBuffer, uint32 BaseVertexIndex, uint32 StartIndex, uint32 PrimitiveCount) = 0;
};

RefCountingPtr<RHIBoundShaderState> RHICreateBoundShaderState(RHIVertexShader* VertexShader, RHIPixelShader* PixelShader);

class RHIContextNonNativePSO : public RHIContext
{
public:
	void RHISetPSO(RHIPipelineStateObject* GraphicsPSO, uint32 StencilRef) override;

	virtual void RHISetBoundShaderState(RHIBoundShaderState* BoundShaderState) = 0;
	virtual void RHISetDepthStencilState(RHIDepthStencilState* State, uint32 StencilRef) = 0;

private:
	void SetPSOFromInitializer(const PipelineStateInitializer& PSOInit, uint32 StencilRef);
};
}
