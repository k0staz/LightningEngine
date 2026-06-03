#include "D3D11StateCache.h"

#include "CoreDefinitions.h"

namespace LE::D3D11
{
void D3D11StateCache::ClearState()
{
	if (DeviceContext)
	{
		DeviceContext->ClearState();
	}

	for (uint32 shaderType = static_cast<uint32>(RHI::ShaderType::Start); shaderType < static_cast<uint32>(RHI::ShaderType::Count); ++shaderType)
	{
		for (uint32 index = 0; index < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++index)
		{
			if (ShaderResourceViews[shaderType][index])
			{
				ShaderResourceViews[shaderType][index]->Release();
				ShaderResourceViews[shaderType][index] = nullptr;
			}
		}
	}

	memset(&CurrentViewport[0], 0, sizeof(D3D11_VIEWPORT) * D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	CurrentCountOfViewports = 0;

	// Depth Stencil State
	CurrentStencilRef = 0;
	CurrentDepthStencilState = nullptr;

	// Shaders
	CurrentVertexShader = nullptr;
	CurrentPixelShader = nullptr;

	// Index buffer
	CurrentIndexBuffer = nullptr;
	CurrentIndexFormat = DXGI_FORMAT_UNKNOWN;
	CurrentIndexOffset = 0;

	CurrentPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	ZeroMemory(CurrentConstantBuffers, sizeof(CurrentConstantBuffers));
}
}
