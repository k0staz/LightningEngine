#pragma once

#include "DynamicRHI.h"
#include "D3D11Utils.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <d3d11.h>
#include <dxgi1_6.h>

#include "D3D11Resources.h"
#include "D3D11ShaderCompiler.h"
#include "D3D11StateCache.h"
#include "D3D11Viewport.h"
#include "RenderResource.h"
#include "RHIContext.h"

using namespace LE::RHI;

namespace LE::D3D11
{
struct D3D11Adapter
{
	D3D11Adapter()
		: DXGIAdapter(nullptr)
	{
	}

	D3D11Adapter(RefCountingPtr<IDXGIAdapter> InDXGIAdapter)
		: DXGIAdapter(InDXGIAdapter)
	{
		if (DXGIAdapter.IsValid())
		{
			VERIFYD3D11RESULT(DXGIAdapter->GetDesc(&DXGIAdapterDesc));
		}
	}

	bool IsValid() const
	{
		return DXGIAdapter != nullptr;
	}

	RefCountingPtr<IDXGIAdapter> DXGIAdapter;
	DXGI_ADAPTER_DESC DXGIAdapterDesc;
};

class D3D11DynamicRHI : public DynamicRHI, public RHIContextNonNativePSO
{
public:
	friend class D3D11Viewport;

	D3D11DynamicRHI(IDXGIFactory1* InDXGIFactory1, const D3D11Adapter& InAdapter);

	virtual RHIInterfaceType GetInterfaceType() const override { return RHIInterfaceType::D3D11; }
	~D3D11DynamicRHI() override;
	void Init() override;
	void ClearState();
	void Shutdown() override;

	static D3D11DynamicRHI& Get() { return *GetDynamicRHI<D3D11DynamicRHI>(); }

	// Dynamic RHI
	RefCountingPtr<RHIConstantBuffer> RHICreateConstantBuffer(const void* Data, const RHIConstantBufferLayout* Layout) override;
	void RHIUpdateConstantBuffer(Renderer::RenderCommandList& CmdList, RHIConstantBuffer* ConstantBuffer, const void* Data) override;
	RefCountingPtr<RHIBuffer> RHICreateBuffer(const RHIBufferDesc& BufferDesc, RHIResourceCreateInfo& CreateInfo) override;
	RefCountingPtr<RHIViewport> RHICreateViewport(void* WindowHandle, uint32 SizeX, uint32 SizeY, bool bIsFullscreen) override;
	RefCountingPtr<RHITexture> RHICreateTexture(const RHITextureDesc& TextureDesc) override;
	RefCountingPtr<RHIDepthStencilState> RHICreateDepthStencilState(const RHIDepthStencilStateDesc& DepthStencilStateDesc) override;
	RefCountingPtr<RHIVertexShader> RHICreateVertexShader(std::span<const uint8> Code) override;
	RefCountingPtr<RHIPixelShader> RHICreatePixelShader(std::span<const uint8> Code) override;
	RefCountingPtr<RHISamplerState> RHICreateSamplerState(const RHISamplerStateInitializer& Initializer) override;
	RefCountingPtr<RHITexture> RHIGetViewportBackBuffer(RHIViewport* Viewport) override;
	RefCountingPtr<RHIReadView> RHICreateReadView(Renderer::RenderCommandList& CmdList, RHIViewableResource* Resource,
		const RHIViewDescription& ViewDescription) override;
	RefCountingPtr<RHIWriteView> RHICreateWriteView(Renderer::RenderCommandList& CmdList, RHIViewableResource* Resource,
		const RHIViewDescription& ViewDescription) override;

	void RHIBeginDrawingViewport(RHIViewport* Viewport) override;
	void RHIEndDrawingViewport(RHIViewport* Viewport) override;
	void RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ) override;
	void RHISetScissorRectangle(bool IsEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY) override;

	void SetRenderTargets(const RHIRenderTargetView* NewRenderTarget,  const RHIDepthRenderTargetView* NewDepthStencilTarget);
	void CommitRenderTargets();
	void CommitUAVs();
	void CommitRenderTargetsAndUAVs();

	RefCountingPtr<RHIBoundShaderState> RHICreateBoundShaderState(RHIVertexShader* VertexShader, RHIPixelShader* PixelShader) override;

	RefCountingPtr<RHIPipelineStateObject> RHICreatePipelineStateObject(const PipelineStateInitializer& Initializer) override;

	RHIContext* RHIGetContext() override;

	// RHI Context
	void RHISetPSO(RHIPipelineStateObject* GraphicsPSO, uint32 StencilRef) override;

	void RHISetBoundShaderState(RHIBoundShaderState* BoundShaderState) override;
	void RHISetDepthStencilState(RHIDepthStencilState* State, uint32 StencilRef) override;
	void RHISetShaderParameters(RHIShader* Shader, const Array<uint8>& ParametersData, const Array<RHIShaderParameter>& Parameters, const Array<RHIShaderParameterResource>& ResourceParameters) override;

	void RHISetShaderParameter(RHIShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 BytesSize, const void* Value) override;
	void RHISetShaderTexture(RHIShader* Shader, uint32 TextureIndex, RHITexture* Texture) override;
	void RHISetShaderReadViewParameter(RHIShader* Shader, uint32 SamplerIndex, RHIReadView* ReadView) override;
	void RHISetShaderWriteViewParameter(RHIShader* Shader, uint32 WriteViewIndex, RHIWriteView* WriteView) override;
	void RHISetShaderSampler(RHIShader* Shader, uint32 SamplerIndex, RHISamplerState* SamplerState) override;
	void RHISetShaderConstantBuffer(RHIShader* Shader, uint32 BufferIndex, RHIConstantBuffer* ConstantBuffer) override;

	void RHIDrawIndexedPrimitive(RHIBuffer* IndexBuffer, uint32 BaseVertexIndex, uint32 StartIndex, uint32 PrimitiveCount) override;

	// D3D11
	template<ShaderType Type>
	void SetShaderResourcesView(D3D11ViewableResource* Resource, ID3D11ShaderResourceView* Srv, uint32 ResourceIndex)
	{
		InternalSetShaderResourceView<Type>(Resource, Srv, ResourceIndex);
	}

	void ClearShaderResource(D3D11ViewableResource* Resource);

	RefCountingPtr<ID3D11Device> GetDevice() const { return Device; }
	RefCountingPtr<IDXGIFactory1> GetFactory() const { return DXGIFactory; }
	RefCountingPtr<ID3D11DeviceContext> GetDeviceContext() const { return ImmediateContext; }

private:
	template<ShaderType Type>
	void InternalSetShaderResourceView(D3D11ViewableResource* Resource, ID3D11ShaderResourceView* Srv, uint32 ResourceIndex);

	template<ShaderType Type>
	void ClearShaderResourceViews(D3D11ViewableResource* Resource);

	template<ShaderType Type>
	void ClearAllShaderResourcesForType();
	void ClearAllShaderResources();

	void SetResourceBoundAsIB(D3D11ViewableResource* Resource);

protected:
	D3D11Texture* CreateD3D11Texture2D(const RHITextureDesc& TextureDesc);


protected:
	RefCountingPtr<ID3D11Device> Device;
	RefCountingPtr<ID3D11DeviceContext> ImmediateContext;
	std::vector<D3D11Viewport*> Viewports;
	RefCountingPtr<D3D11Viewport> CurrentViewport;
	RefCountingPtr<ID3D11DepthStencilView> CurrentDeptStencilTarget;
	RefCountingPtr<D3D11Texture> CurrentDepthTexture;
	RefCountingPtr<IDXGIFactory1> DXGIFactory;
	ExclusiveDepthStencil CurrentDSAccessType;
	D3D11Adapter Adapter;
	D3D_FEATURE_LEVEL FeatureLevel;

	PrimitiveType CurrentPrimitiveType;

	RefCountingPtr<ID3D11RenderTargetView> CurrentRenderTarget;
	RefCountingPtr<D3D11WriteView> CurrentWriteViews[D3D11_PS_CS_UAV_REGISTER_COUNT];
	ID3D11UnorderedAccessView* UAVBound[D3D11_PS_CS_UAV_REGISTER_COUNT];
	uint32 UAVFirstBind;
	uint32 UAVBindCount;
	bool UAVChanged;

	Renderer::BoundShaderStateCash<100> BoundShaderStateHistory;

	D3D11StateCache StateCache;

	D3D11ViewableResource* ResourcesBoundAsReadViews[static_cast<uint8>(ShaderType::Count)][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	D3D11ViewableResource* ResourceBoundAsIndexBuffer;

	D3D11ConstantBuffer* BoundConstantBuffers[static_cast<uint8>(ShaderType::Count)][D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];

	int32 MaxBoundShaderResourcesIndex[static_cast<uint8>(ShaderType::Count)];

	uint32 FrameCounter;
};

class D3D11DynamicRHIModule : public DynamicRHIModule
{
public:
	DynamicRHI* CreateRHI() override;
};

static D3D11DynamicRHIModule gD3D11Module;

// This is temp, once we have more backends we need to make them DLLs and load at runtime
inline void UseD3D11RHIModule()
{
	RegisterRHIModule(&gD3D11Module);
	UseD3D11ShaderCompiler();
}
}
