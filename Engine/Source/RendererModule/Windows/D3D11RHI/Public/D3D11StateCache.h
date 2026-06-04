#pragma once
#include <d3d11.h>

#include "Core.h"
#include "CoreDefinitions.h"
#include "RHIDefinitions.h"
#include "Math/Math.h"

namespace LE::D3D11
{
class D3D11StateCache
{
public:
	D3D11StateCache()
		: DeviceContext(nullptr)
	{
		ZeroMemory(ShaderResourceViews, sizeof(ShaderResourceViews));
		ZeroMemory(CurrentSamplerStates, sizeof(CurrentSamplerStates));
	}

	void Init(ID3D11DeviceContext* InDeviceContext)
	{
		DeviceContext = InDeviceContext;
		ClearState();
	}

	void ClearState();

	void GetViewport(D3D11_VIEWPORT* Viewport)
	{
		LE_ASSERT(Viewport)
		memcpy(Viewport, &CurrentViewport, sizeof(D3D11_VIEWPORT));
	}

	void GetViewports(uint32& Count, D3D11_VIEWPORT* Viewports)
	{
		if (Viewports)
		{
			uint32 sizeCount = Count;
			uint32 copyCount = LE::Min(LE::Min(sizeCount, CurrentCountOfViewports),
			                           static_cast<uint32>(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));
			if (copyCount > 0)
			{
				memcpy(Viewports, &CurrentViewport[0], sizeof(D3D11_VIEWPORT) * copyCount);
			}

			if (sizeCount > copyCount)
			{
				memset(&Viewports[copyCount], 0, sizeof(D3D11_VIEWPORT) * (sizeCount - copyCount));
			}
		}

		Count = CurrentCountOfViewports;
	}

	void SetViewport(D3D11_VIEWPORT Viewport)
	{
		if (CurrentCountOfViewports != 1 || memcmp(&CurrentViewport[0], &Viewport, sizeof(D3D11_VIEWPORT)))
		{
			memcpy(&CurrentViewport[0], &Viewport, sizeof(D3D11_VIEWPORT));
			CurrentCountOfViewports = 1;
			DeviceContext->RSSetViewports(1, &Viewport);
		}
	}

	template <RHI::ShaderType Type>
	void SetShaderResourceView(ID3D11ShaderResourceView* Srv, uint32 ResourceIndex)
	{
		InternalSetShaderResourceView<Type>(Srv, ResourceIndex);
	}

	template <RHI::ShaderType Type>
	void ClearConstantBuffers()
	{
		ZeroMemory(CurrentConstantBuffers[static_cast<uint8>(Type)], sizeof(CurrentConstantBuffers[static_cast<uint8>(Type)]));

		ID3D11Buffer* emptyBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {nullptr};
		switch (Type)
		{
		case RHI::ShaderType::Vertex:
			DeviceContext->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, emptyBuffers);
			break;
		case RHI::ShaderType::Pixel:
			DeviceContext->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, emptyBuffers);
			break;
		}
	}

	template <RHI::ShaderType Type>
	void SetSamplerState(ID3D11SamplerState* SamplerState, uint32 SamplerIndex)
	{
		InternalSetSamplerState<Type>(SamplerState, SamplerIndex);
	}

	template <RHI::ShaderType Type>
	void SetConstantBuffer(ID3D11Buffer* ConstantBuffer, uint32 SlotIndex)
	{
		D3D11ConstantBufferState& current = CurrentConstantBuffers[static_cast<uint32>(Type)][SlotIndex];
		if (current.ConstantBuffer != ConstantBuffer)
		{
			current.ConstantBuffer = ConstantBuffer;
			InternalSetConstantBuffer<Type>(ConstantBuffer, SlotIndex);
		}
	}

	void SetVertexShader(ID3D11VertexShader* Shader)
	{
		if (CurrentVertexShader != Shader)
		{
			CurrentVertexShader = Shader;
			DeviceContext->VSSetShader(Shader, nullptr, 0);
		}
	}

	void GetVertexShader(ID3D11VertexShader** Shader)
	{
		*Shader = CurrentVertexShader;
		if (CurrentVertexShader)
		{
			CurrentVertexShader->AddRef();
		}
	}

	void SetPixelShader(ID3D11PixelShader* Shader)
	{
		if (CurrentPixelShader != Shader)
		{
			CurrentPixelShader = Shader;
			DeviceContext->PSSetShader(Shader, nullptr, 0);
		}
	}

	void GetPixelShader(ID3D11PixelShader** Shader)
	{
		*Shader = CurrentPixelShader;
		if (CurrentPixelShader)
		{
			CurrentPixelShader->AddRef();
		}
	}

	void SetIndexBuffer(ID3D11Buffer* IndexBuffer, DXGI_FORMAT Format, uint32 Offset)
	{
		if (IndexBuffer != CurrentIndexBuffer || Format != CurrentIndexFormat || Offset != CurrentIndexOffset)
		{
			CurrentIndexBuffer = IndexBuffer;
			CurrentIndexFormat = Format;
			CurrentIndexOffset = Offset;

			DeviceContext->IASetIndexBuffer(IndexBuffer, Format, Offset);
		}
	}

	void GetIndexBuffer(ID3D11Buffer** IndexBuffer, DXGI_FORMAT* Format, uint32* Offset)
	{
		if (IndexBuffer)
		{
			*IndexBuffer = CurrentIndexBuffer;
			if (CurrentIndexBuffer)
			{
				CurrentIndexBuffer->AddRef();
			}
		}

		if (Format)
		{
			*Format = CurrentIndexFormat;
		}

		if (Offset)
		{
			*Offset = CurrentIndexOffset;
		}
	}

	void SetDepthStencilState(ID3D11DepthStencilState* State, uint32 RefStencil)
	{
		if (State != CurrentDepthStencilState || RefStencil != CurrentStencilRef)
		{
			CurrentStencilRef = RefStencil;
			CurrentDepthStencilState = State;
			DeviceContext->OMSetDepthStencilState(State, RefStencil);
		}
	}

	void SetDepthStencilRef(uint32 RefStencil)
	{
		if (RefStencil != CurrentStencilRef)
		{
			CurrentStencilRef = RefStencil;
			DeviceContext->OMSetDepthStencilState(CurrentDepthStencilState, RefStencil);
		}
	}

	void GetDepthStencilState(ID3D11DepthStencilState** DepthStencilState, uint32* RefStencil)
	{
		if (DepthStencilState)
		{
			*DepthStencilState = CurrentDepthStencilState;
			if (CurrentDepthStencilState)
			{
				CurrentDepthStencilState->AddRef();
			}
		}

		if (RefStencil)
		{
			*RefStencil = CurrentStencilRef;
		}
	}

	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology)
	{
		if (CurrentPrimitiveTopology != PrimitiveTopology)
		{
			CurrentPrimitiveTopology = PrimitiveTopology;
			DeviceContext->IASetPrimitiveTopology(PrimitiveTopology);
		}
	}

	void GetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY* PrimitiveTopology)
	{
		*PrimitiveTopology = CurrentPrimitiveTopology;
	}

private:
	template <RHI::ShaderType Type>
	void InternalSetShaderResourceView(ID3D11ShaderResourceView*& Srv, uint32 ResourceIndex)
	{
		if (ShaderResourceViews[static_cast<uint8>(Type)][ResourceIndex] != Srv)
		{
			if (Srv)
			{
				Srv->AddRef();
			}

			if (ShaderResourceViews[static_cast<uint8>(Type)][ResourceIndex])
			{
				ShaderResourceViews[static_cast<uint8>(Type)][ResourceIndex]->Release();
			}

			ShaderResourceViews[static_cast<uint8>(Type)][ResourceIndex] = Srv;
			switch (Type)
			{
			case RHI::ShaderType::Vertex:
				DeviceContext->VSSetShaderResources(ResourceIndex, 1, &Srv);
				break;
			case RHI::ShaderType::Pixel:
				DeviceContext->PSSetShaderResources(ResourceIndex, 1, &Srv);
				break;
			}
		}
	}

	template <RHI::ShaderType Type>
	void InternalSetSamplerState(ID3D11SamplerState* SamplerState, uint32 SamplerIndex)
	{
		LE_ASSERT(SamplerIndex < ARRAYSIZE(CurrentSamplerStates[static_cast<uint32>(Type)]))
		if (CurrentSamplerStates[static_cast<uint32>(Type)][SamplerIndex] != SamplerState)
		{
			CurrentSamplerStates[static_cast<uint32>(Type)][SamplerIndex] = SamplerState;
			switch (Type)
			{
			case RHI::ShaderType::Vertex:
				DeviceContext->VSSetSamplers(SamplerIndex, 1, &SamplerState);
				break;
			case RHI::ShaderType::Pixel:
				DeviceContext->PSSetSamplers(SamplerIndex, 1, &SamplerState);
				break;
			}
		}
	}

	template <RHI::ShaderType Type>
	void InternalSetConstantBuffer(ID3D11Buffer* ConstantBuffer, uint32 SlotIndex)
	{
		switch (Type)
		{
		case RHI::ShaderType::Vertex:
			DeviceContext->VSSetConstantBuffers(SlotIndex, 1, &ConstantBuffer);
			break;
		case RHI::ShaderType::Pixel:
			DeviceContext->PSSetConstantBuffers(SlotIndex, 1, &ConstantBuffer);
			break;
		}
	}

private:
	ID3D11DeviceContext* DeviceContext;

	ID3D11ShaderResourceView* ShaderResourceViews[static_cast<uint8>(RHI::ShaderType::Count)][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];

	uint32 CurrentCountOfViewports;
	D3D11_VIEWPORT CurrentViewport[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

	// Depth Stencil State
	uint32 CurrentStencilRef;
	ID3D11DepthStencilState* CurrentDepthStencilState;

	// Shaders
	ID3D11VertexShader* CurrentVertexShader;
	ID3D11PixelShader* CurrentPixelShader;

	// Index buffer
	ID3D11Buffer* CurrentIndexBuffer;
	DXGI_FORMAT CurrentIndexFormat;
	uint32 CurrentIndexOffset;

	// Primitive Topology
	D3D11_PRIMITIVE_TOPOLOGY CurrentPrimitiveTopology;

	// Sampler State
	ID3D11SamplerState* CurrentSamplerStates[static_cast<uint8>(RHI::ShaderType::Count)][D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

	struct D3D11ConstantBufferState
	{
		ID3D11Buffer* ConstantBuffer;
	} CurrentConstantBuffers[static_cast<uint8>(RHI::ShaderType::Count)][D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
};
}
