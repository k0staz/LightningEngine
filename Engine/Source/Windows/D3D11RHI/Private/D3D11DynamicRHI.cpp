#include "D3D11DynamicRHI.h"

#include "D3D11Resources.h"
#include "D3D11Viewport.h"
#include "Shader.h"
#include "Templates/Alignment.h"

namespace LE::D3D11
{
static D3D11_COMPARISON_FUNC GetComparisonFunc(CompareFunction CompareFunction)
{
	switch (CompareFunction)
	{
	case CompareFunction::Less:
		return D3D11_COMPARISON_LESS;
	case CompareFunction::LessEqual:
		return D3D11_COMPARISON_LESS_EQUAL;
	case CompareFunction::Greater:
		return D3D11_COMPARISON_GREATER;
	case CompareFunction::GreaterEqual:
		return D3D11_COMPARISON_GREATER_EQUAL;
	case CompareFunction::Equal:
		return D3D11_COMPARISON_EQUAL;
	case CompareFunction::NotEqual:
		return D3D11_COMPARISON_NOT_EQUAL;
	case CompareFunction::Never:
		return D3D11_COMPARISON_NEVER;
	case CompareFunction::Always:
	case CompareFunction::Count:
		return D3D11_COMPARISON_ALWAYS;
	}
	return D3D11_COMPARISON_ALWAYS;
}

static D3D11_STENCIL_OP GetStencilOp(StencilOp StencilOp)
{
	switch (StencilOp)
	{
	case StencilOp::Zero:
		return D3D11_STENCIL_OP_ZERO;
	case StencilOp::Replace:
		return D3D11_STENCIL_OP_REPLACE;
	case StencilOp::SaturatedIncrement:
		return D3D11_STENCIL_OP_INCR_SAT;
	case StencilOp::SaturatedDecrement:
		return D3D11_STENCIL_OP_DECR_SAT;
	case StencilOp::Invert:
		return D3D11_STENCIL_OP_INVERT;
	case StencilOp::Increment:
		return D3D11_STENCIL_OP_INCR;
	case StencilOp::Decrement:
		return D3D11_STENCIL_OP_DECR;
	case StencilOp::Keep:
	case StencilOp::Count:
		return D3D11_STENCIL_OP_KEEP;
	}
	return D3D11_STENCIL_OP_KEEP;
}

static D3D11_TEXTURE_ADDRESS_MODE GetTextureAddressMode(SamplerAddressMode AddressMode)
{
	switch (AddressMode)
	{
	case SamplerAddressMode::Clamp:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case SamplerAddressMode::Mirror:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
	case SamplerAddressMode::Border:
		return D3D11_TEXTURE_ADDRESS_BORDER;
	case SamplerAddressMode::Count:
	case SamplerAddressMode::Wrap:
	default:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

struct RTVDescription
{
	uint32 Width;
	uint32 Height;
	DXGI_SAMPLE_DESC SampleDesc;
};

static RTVDescription GetRenderTargetViewDescription(ID3D11RenderTargetView* RenderTargetView)
{
	D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc;
	RenderTargetView->GetDesc(&targetViewDesc);

	RefCountingPtr<ID3D11Resource> baseResource;
	RenderTargetView->GetResource(baseResource.GetInitPointer());

	RTVDescription result;
	switch (targetViewDesc.ViewDimension)
	{
	case D3D11_RTV_DIMENSION_TEXTURE2D:
	case D3D11_RTV_DIMENSION_TEXTURE2DARRAY:
	case D3D11_RTV_DIMENSION_TEXTURE2DMS:
	case D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY:
		{
			D3D11_TEXTURE2D_DESC desc;
			ID3D11Texture2D* texture2D = nullptr;
			LE_ASSERT(SUCCEEDED(baseResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture2D))) && texture2D)

			texture2D->GetDesc(&desc);
			result.Width = desc.Width;
			result.Height = desc.Height;
			result.SampleDesc = desc.SampleDesc;

			texture2D->Release();

			break;
		}
	case D3D11_RTV_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			ID3D11Texture3D* texture3D = nullptr;
			LE_ASSERT(SUCCEEDED(baseResource->QueryInterface(__uuidof(ID3D11Texture3D), reinterpret_cast<void**>(&texture3D))) && texture3D)

			texture3D->GetDesc(&desc);
			result.Width = desc.Width;
			result.Height = desc.Height;
			result.SampleDesc.Count = 1;
			result.SampleDesc.Quality = 0;

			texture3D->Release();

			break;
		}
	default:
		{
			LE_ASSERT(false)
		}
	}

	return result;
}

static D3D11_PRIMITIVE_TOPOLOGY GetD3D11PrimitiveTopology(PrimitiveType Primitive)
{
	switch (Primitive)
	{
	case PrimitiveType::TriangleList:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case PrimitiveType::TriangleStrip:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case PrimitiveType::LineList:
		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case PrimitiveType::PointList:
		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	}

	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

D3D11DynamicRHI::D3D11DynamicRHI(IDXGIFactory1* InDXGIFactory1, const D3D11Adapter& InAdapter)
	: DXGIFactory(InDXGIFactory1)
{
	Adapter = InAdapter;
	FeatureLevel = D3D_FEATURE_LEVEL_11_0;
}

D3D11DynamicRHI::~D3D11DynamicRHI()
{
}

void D3D11DynamicRHI::Init()
{
	if (Device.IsValid())
	{
		return;
	}

	{
		ClearState();
		uint32 createFlags = 0;
#if DEBUG
		createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		VERIFYD3D11RESULT(D3D11CreateDevice(
			Adapter.DXGIAdapter.GetPointer(),
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			createFlags,
			&FeatureLevel,
			1,
			D3D11_SDK_VERSION,
			Device.GetInitPointer(),
			nullptr,
			ImmediateContext.GetInitPointer()))


		StateCache.Init(ImmediateContext.GetPointer());
	}
}

void D3D11DynamicRHI::ClearState()
{
	StateCache.ClearState();

	ZeroMemory(ResourcesBoundAsReadViews, sizeof(ResourcesBoundAsReadViews));
	for (uint32 shaderType = static_cast<uint32>(RHI::ShaderType::Start); shaderType < static_cast<uint32>(RHI::ShaderType::Count); ++
	     shaderType)
	{
		MaxBoundShaderResourcesIndex[shaderType] = -1;
	}

	ZeroMemory(BoundConstantBuffers, sizeof(BoundConstantBuffers));
}

void D3D11DynamicRHI::Shutdown()
{
}

RefCountingPtr<RHIConstantBuffer> D3D11DynamicRHI::RHICreateConstantBuffer(const void* Data, const RHIConstantBufferLayout* Layout)
{
	D3D11ConstantBuffer* newConstantBuffer = nullptr;
	const uint32 bufferSize = Layout->ConstantBufferSize;
	if (bufferSize > 0)
	{
		LE_ASSERT(Align(bufferSize, 16) == bufferSize)
		LE_ASSERT(Align(Data, 16) == Data)

		D3D11_BUFFER_DESC description;
		ZeroMemory(&description, sizeof(D3D11_BUFFER_DESC));

		description.ByteWidth = bufferSize;
		description.Usage = D3D11_USAGE_DYNAMIC;
		description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		description.MiscFlags = 0;
		description.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA immutableData;
		immutableData.pSysMem = Data;
		immutableData.SysMemPitch = 0;
		immutableData.SysMemSlicePitch = 0;

		RefCountingPtr<ID3D11Buffer> constantBufferResource;
		VERIFYD3D11RESULT(Device->CreateBuffer(&description, Data ? &immutableData : nullptr, constantBufferResource.GetInitPointer()))

		newConstantBuffer = new D3D11ConstantBuffer(Layout, constantBufferResource);
	}
	else
	{
		newConstantBuffer = new D3D11ConstantBuffer(Layout, nullptr);
	}

	const uint32 resourceCount = static_cast<uint32>(Layout->Resources.size());
	if (resourceCount > 0)
	{
		Array<RefCountingPtr<RHIResource>>& resources = const_cast<Array<RefCountingPtr<RHIResource>>&>(newConstantBuffer->GetResources());
		resources.clear();
		resources.reserve(resourceCount);

		if (Data)
		{
			for (uint32 index = 0; index < resourceCount; ++index)
			{
				const auto resourceParameter = Layout->Resources[index];
				resources.emplace_back(
					Renderer::GetShaderParameterResourceRHI(Data, resourceParameter.ResourceOffset, resourceParameter.ResourceType));
			}
		}
	}

	return newConstantBuffer;
}

void D3D11DynamicRHI::RHIUpdateConstantBuffer(Renderer::RenderCommandList& CmdList, RHIConstantBuffer* ConstantBuffer, const void* Data)
{
	LE_ASSERT(ConstantBuffer)

	D3D11ConstantBuffer* buffer = ResourceCast(ConstantBuffer);

	const RHIConstantBufferLayout& layout = ConstantBuffer->GetLayout();
	const uint32 bufferSize = layout.ConstantBufferSize;
	if (bufferSize == 0)
	{
		return;
	}

	// TODO:: Resources update, we need to implement memory stack on cmd list first
	/*

	const uint32 resourcesCount = layout.Resources.Count();
	LE_ASSERT(buffer->GetResources().Count() == resourcesCount)

	RHIResource** cmdListResources = nullptr;
	void* cmdListBufferData = nullptr;

	if (resourcesCount > 0)
	{

	}*/


	CmdList.EnqueueLambdaCommand(
		[Context = ImmediateContext.GetPointer(), Device = Device.GetPointer(), bufferSize, buffer, Data](Renderer::RenderCommandList&)
		{
			LE_ASSERT(Align(Data, 16) == Data)

			D3D11_MAPPED_SUBRESOURCE subresource;
			VERIFYD3D11RESULT(Context->Map(buffer->GetResource().GetPointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource))
			LE_ASSERT(subresource.RowPitch >= bufferSize)

			memcpy(subresource.pData, Data, bufferSize);
			Context->Unmap(buffer->GetResource().GetPointer(), 0);
		});
}

RefCountingPtr<RHIBuffer> D3D11DynamicRHI::RHICreateBuffer(const RHIBufferDesc& BufferDesc, RHIResourceCreateInfo& CreateInfo)
{
	if (BufferDesc.IsNull())
	{
		return new D3D11Buffer(nullptr, BufferDesc);
	}

	LE_ASSERT_DESC(BufferDesc.Size > 0, "Buffer Size is 0, this is not allowed")

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = BufferDesc.Size;

	if (EnumHasAnyFlags(BufferDesc.Usage, BUF_Vertex))
	{
		desc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
	}

	if (EnumHasAnyFlags(BufferDesc.Usage, BUF_Index))
	{
		desc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
	}

	if (EnumHasAnyFlags(BufferDesc.Usage, BUF_ShaderResource))
	{
		desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}

	D3D11_SUBRESOURCE_DATA initData;
	D3D11_SUBRESOURCE_DATA* initDataPtr = nullptr;
	if (CreateInfo.ResourceArray)
	{
		LE_ASSERT(BufferDesc.Size == CreateInfo.ResourceArray->GetResourceDataSize())
		initData.pSysMem = CreateInfo.ResourceArray->GetResourceData();
		initData.SysMemPitch = BufferDesc.Size;
		initData.SysMemSlicePitch = 0;
		initDataPtr = &initData;
	}

	RefCountingPtr<ID3D11Buffer> bufferResource;
	VERIFYD3D11RESULT(Device->CreateBuffer(&desc, initDataPtr, bufferResource.GetInitPointer()))

	if (CreateInfo.ResourceArray)
	{
		CreateInfo.ResourceArray->Discard();
	}

	return new D3D11Buffer(bufferResource, BufferDesc);
}

RefCountingPtr<RHIViewport> D3D11DynamicRHI::RHICreateViewport(void* WindowHandle, uint32 SizeX, uint32 SizeY, bool bIsFullscreen)
{
	return new D3D11Viewport(this, static_cast<HWND>(WindowHandle), SizeX, SizeY, bIsFullscreen);
}

RefCountingPtr<RHITexture> D3D11DynamicRHI::RHICreateTexture(const RHITextureDesc& TextureDesc)
{
	LE_ASSERT_DESC(TextureDesc.Dimension == TextureDimensions::Texture2D, "Texture 3D is not yet implemented")

	return TextureDesc.Dimension == TextureDimensions::Texture2D ? RefCountingPtr<RHITexture>(CreateD3D11Texture2D(TextureDesc)) : nullptr;
}

RefCountingPtr<RHIDepthStencilState> D3D11DynamicRHI::RHICreateDepthStencilState(const RHIDepthStencilStateDesc& DepthStencilStateDesc)
{
	D3D11DepthStencilState* depthStencilState = new D3D11DepthStencilState;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthStencilDesc.DepthEnable = DepthStencilStateDesc.DepthTest != CompareFunction::Always || DepthStencilStateDesc.bEnableDepthWrite;
	depthStencilDesc.DepthWriteMask = DepthStencilStateDesc.bEnableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = GetComparisonFunc(DepthStencilStateDesc.DepthTest);

	depthStencilDesc.StencilEnable = DepthStencilStateDesc.bEnableFrontFaceStencil || DepthStencilStateDesc.bEnableBackFaceStencil;
	depthStencilDesc.StencilReadMask = DepthStencilStateDesc.StencilReadMask;
	depthStencilDesc.StencilWriteMask = DepthStencilStateDesc.StencilWriteMask;
	depthStencilDesc.FrontFace.StencilFunc = GetComparisonFunc(DepthStencilStateDesc.FrontFaceStencilTest);
	depthStencilDesc.FrontFace.StencilFailOp = GetStencilOp(DepthStencilStateDesc.FrontFaceStencilFailOp);
	depthStencilDesc.FrontFace.StencilDepthFailOp = GetStencilOp(DepthStencilStateDesc.FrontFaceDepthFailOp);
	depthStencilDesc.FrontFace.StencilPassOp = GetStencilOp(DepthStencilStateDesc.FrontFacePassFailOp);
	if (DepthStencilStateDesc.bEnableBackFaceStencil)
	{
		depthStencilDesc.BackFace.StencilFunc = GetComparisonFunc(DepthStencilStateDesc.BackFaceStencilTest);
		depthStencilDesc.BackFace.StencilFailOp = GetStencilOp(DepthStencilStateDesc.BackFaceStencilFailOp);
		depthStencilDesc.BackFace.StencilDepthFailOp = GetStencilOp(DepthStencilStateDesc.BackFaceDepthFailOp);
		depthStencilDesc.BackFace.StencilPassOp = GetStencilOp(DepthStencilStateDesc.BackFacePassFailOp);
	}
	else
	{
		depthStencilDesc.BackFace = depthStencilDesc.FrontFace;
	}

	const bool stencilOpIsKeep =
		DepthStencilStateDesc.FrontFaceStencilFailOp == StencilOp::Keep
		&& DepthStencilStateDesc.FrontFaceDepthFailOp == StencilOp::Keep
		&& DepthStencilStateDesc.FrontFacePassFailOp == StencilOp::Keep
		&& DepthStencilStateDesc.BackFaceStencilFailOp == StencilOp::Keep
		&& DepthStencilStateDesc.BackFaceDepthFailOp == StencilOp::Keep
		&& DepthStencilStateDesc.BackFacePassFailOp == StencilOp::Keep;

	const bool mayWriteStencil = DepthStencilStateDesc.StencilWriteMask != 0 && !stencilOpIsKeep;
	depthStencilState->GetAccessType().SetDepthStencilWrite(DepthStencilStateDesc.bEnableDepthWrite, mayWriteStencil);

	VERIFYD3D11RESULT(Device->CreateDepthStencilState(&depthStencilDesc, depthStencilState->GetResource().GetInitPointer()))
	return depthStencilState;
}

RefCountingPtr<RHIVertexShader> D3D11DynamicRHI::RHICreateVertexShader(std::span<const uint8> Code)
{
	D3D11VertexShader* shader = new D3D11VertexShader;
	VERIFYD3D11RESULT(Device->CreateVertexShader(Code.data(), Code.size(), nullptr, shader->GetResource().GetInitPointer()))
	return shader;
}

RefCountingPtr<RHIPixelShader> D3D11DynamicRHI::RHICreatePixelShader(std::span<const uint8> Code)
{
	D3D11PixelShader* shader = new D3D11PixelShader;
	VERIFYD3D11RESULT(Device->CreatePixelShader(Code.data(), Code.size(), nullptr, shader->GetResource().GetInitPointer()))
	return shader;
}

RefCountingPtr<RHISamplerState> D3D11DynamicRHI::RHICreateSamplerState(const RHISamplerStateInitializer& Initializer)
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));

	desc.AddressU = GetTextureAddressMode(Initializer.AddressU);
	desc.AddressV = GetTextureAddressMode(Initializer.AddressV);
	desc.AddressW = GetTextureAddressMode(Initializer.AddressW);
	desc.MipLODBias = Initializer.MipMapBias;
	desc.MaxAnisotropy = Initializer.MaxAnisotropy;
	desc.MinLOD = Initializer.MinMipMapLevel;
	desc.MaxLOD = Initializer.MaxMipMapLevel;

	const bool isComparisonEnabled = Initializer.SamplerCompareFunction != SamplerCompareFunction::Never;
	switch (Initializer.Filter)
	{
	case SamplerFilter::Point:
		desc.Filter = isComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	case SamplerFilter::Bilinear:
		desc.Filter = isComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case SamplerFilter::Trilinear:
		desc.Filter = isComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case SamplerFilter::Anisotropic:
		if (desc.MaxAnisotropy == 1)
		{
			desc.Filter = isComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}
		else
		{
			desc.Filter = isComparisonEnabled ? D3D11_FILTER_COMPARISON_ANISOTROPIC : D3D11_FILTER_ANISOTROPIC;
		}
		break;
	case SamplerFilter::Count:
		break;
	}

	desc.BorderColor[0] = Initializer.BorderColor.R;
	desc.BorderColor[1] = Initializer.BorderColor.G;
	desc.BorderColor[2] = Initializer.BorderColor.B;
	desc.BorderColor[3] = Initializer.BorderColor.A;

	D3D11SamplerState* newSamplerState = new D3D11SamplerState;
	VERIFYD3D11RESULT(Device->CreateSamplerState(&desc, newSamplerState->GetResource().GetInitPointer()))

	return newSamplerState;
}

RefCountingPtr<RHITexture> D3D11DynamicRHI::RHIGetViewportBackBuffer(RHIViewport* Viewport)
{
	D3D11Viewport* d3d11Viewport = ResourceCast(Viewport);

	return d3d11Viewport->GetBackBuffer();
}

RefCountingPtr<RHIReadView> D3D11DynamicRHI::RHICreateReadView(Renderer::RenderCommandList& CmdList, RHIViewableResource* Resource,
                                                               const RHIViewDescription& ViewDescription)
{
	return new D3D11ReadView(CmdList, Resource, ViewDescription);
}

RefCountingPtr<RHIWriteView> D3D11DynamicRHI::RHICreateWriteView(Renderer::RenderCommandList& CmdList, RHIViewableResource* Resource,
                                                                 const RHIViewDescription& ViewDescription)
{
	return new D3D11WriteView(CmdList, Resource, ViewDescription);
}

void D3D11DynamicRHI::RHIBeginDrawingViewport(RHIViewport* Viewport)
{
	D3D11Viewport* d3d11Viewport = ResourceCast(Viewport);

	LE_ASSERT(!CurrentViewport)
	CurrentViewport = d3d11Viewport;
	RHIRenderTargetView renderTarget(d3d11Viewport->GetBackBuffer(), RenderTargetLoadAction::Load);
	SetRenderTargets(&renderTarget, nullptr);

	RHISetScissorRectangle(false, 0, 0, 0, 0);
}

void D3D11DynamicRHI::RHIEndDrawingViewport(RHIViewport* Viewport)
{
	++FrameCounter;

	LE_ASSERT(Viewport == CurrentViewport.GetPointer())
	CurrentViewport = nullptr;

	CurrentDeptStencilTarget = nullptr;
	CurrentDSAccessType = ExclusiveDepthStencil::DepthWrite_StencilWrite;
	CurrentDepthTexture = nullptr;
	CurrentRenderTarget = nullptr;

	ClearAllShaderResources();

	CommitRenderTargetsAndUAVs();

	StateCache.SetVertexShader(nullptr);
	StateCache.SetPixelShader(nullptr);

	StateCache.SetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
	ResourceBoundAsIndexBuffer = nullptr;

	D3D11Viewport* d3d11Viewport = ResourceCast(Viewport);
	d3d11Viewport->Present();
}

void D3D11DynamicRHI::RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ)
{
	LE_ASSERT(MinX <= static_cast<float>(D3D11_VIEWPORT_BOUNDS_MAX))
	LE_ASSERT(MinY <= static_cast<float>(D3D11_VIEWPORT_BOUNDS_MAX))
	LE_ASSERT(MaxX <= static_cast<float>(D3D11_VIEWPORT_BOUNDS_MAX))
	LE_ASSERT(MaxY <= static_cast<float>(D3D11_VIEWPORT_BOUNDS_MAX))

	D3D11_VIEWPORT viewport = {MinX, MinY, MaxX - MinX, MaxY - MinY, MinZ, MaxZ};
	StateCache.SetViewport(viewport);
	RHISetScissorRectangle(true, static_cast<uint32>(MinX), static_cast<uint32>(MinY), static_cast<uint32>(MaxX), static_cast<uint32>(MaxY));
}

void D3D11DynamicRHI::RHISetScissorRectangle(bool IsEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY)
{
	D3D11_VIEWPORT viewport;
	StateCache.GetViewport(&viewport);

	D3D11_RECT scissorRect;
	if (IsEnable)
	{
		scissorRect.left = static_cast<LONG>(MinX);
		scissorRect.top = static_cast<LONG>(MinY);
		scissorRect.right = static_cast<LONG>(MaxX);
		scissorRect.bottom = static_cast<LONG>(MaxY);
	}
	else
	{
		scissorRect.left = static_cast<LONG>(viewport.TopLeftX);
		scissorRect.top = static_cast<LONG>(viewport.TopLeftY);
		scissorRect.right = static_cast<LONG>(viewport.TopLeftX + viewport.Width);
		scissorRect.bottom = static_cast<LONG>(viewport.TopLeftY + viewport.Height);
	}

	ImmediateContext->RSSetScissorRects(1, &scissorRect);
}

void D3D11DynamicRHI::SetRenderTargets(const RHIRenderTargetView* NewRenderTarget,
                                       const RHIDepthRenderTargetView* NewDepthStencilTarget)
{
	D3D11Texture* depthStencilTarget = ResourceCast(NewDepthStencilTarget ? NewDepthStencilTarget->Texture : nullptr);

	bool isTargetChanged = false;

	ID3D11DepthStencilView* depthStencilView = nullptr;
	if (depthStencilTarget)
	{
		CurrentDSAccessType = NewDepthStencilTarget->GetDepthStencilAccess();
		depthStencilView = depthStencilTarget->GetDepthStencilView(CurrentDSAccessType);

		ClearShaderResource(depthStencilTarget);
	}

	if (CurrentDeptStencilTarget.GetPointer() != depthStencilView)
	{
		CurrentDepthTexture = depthStencilTarget;
		CurrentDeptStencilTarget = depthStencilView;
		isTargetChanged = true;
	}

	ID3D11RenderTargetView* renderTargetView = nullptr;
	if (NewRenderTarget->Texture != nullptr)
	{
		D3D11Texture* newRenderTarget = ResourceCast(NewRenderTarget->Texture);
		renderTargetView = newRenderTarget->GetRenderTargetView(0);

		ClearShaderResource(newRenderTarget);
		// TODO: REMOVE THIS ONCE RENDER GRAPH IS IMPLEMENTED, IT SHOULD BE CLEANED ON RENDER PASS BEGIN VIA RENDER PASS INFO
		float color[4];
		color[0] = 0.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		color[3] = 0.0f;
		ImmediateContext->ClearRenderTargetView(renderTargetView, color);
	}

	if (CurrentRenderTarget.GetPointer() != renderTargetView)
	{
		CurrentRenderTarget = renderTargetView;
		isTargetChanged = true;
	}

	if (isTargetChanged)
	{
		CommitRenderTargets();
	}

	if (renderTargetView)
	{
		RTVDescription description = GetRenderTargetViewDescription(renderTargetView);
		RHISetViewport(0.0f, 0.0f, 0.0f, static_cast<float>(description.Width), static_cast<float>(description.Height), 1.0f);
	}
	else if (depthStencilView)
	{
		RefCountingPtr<ID3D11Resource> baseResource;
		depthStencilView->GetResource(baseResource.GetInitPointer());

		D3D11_TEXTURE2D_DESC description;
		dynamic_cast<ID3D11Texture2D*>(baseResource.GetPointer())->GetDesc(&description);
		RHISetViewport(0.0f, 0.0f, 0.0f, static_cast<float>(description.Width), static_cast<float>(description.Height), 1.0f);
	}
}

void D3D11DynamicRHI::CommitRenderTargets()
{
	ID3D11RenderTargetView* rtArray[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	rtArray[0] = CurrentRenderTarget; // Only one for now
	ImmediateContext->OMSetRenderTargets(1, rtArray, CurrentDeptStencilTarget);
}

void D3D11DynamicRHI::CommitUAVs()
{
	if (!UAVChanged)
	{
		return;
	}

	bool found = false;
	uint32 first = 0;
	uint32 count = 0;
	for (uint32 i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; ++i)
	{
		if (CurrentWriteViews[i] != nullptr)
		{
			found = true;
			first = i;
			break;
		}
	}

	if (found)
	{
		D3D11WriteView* writeViews[D3D11_PS_CS_UAV_REGISTER_COUNT];
		ID3D11UnorderedAccessView* uavs[D3D11_PS_CS_UAV_REGISTER_COUNT] = {nullptr};

		for (uint32 i = first; i < D3D11_PS_CS_UAV_REGISTER_COUNT; ++i)
		{
			if (CurrentWriteViews[i] == nullptr)
				break;

			writeViews[i] = CurrentWriteViews[i].GetPointer();
			uavs[i] = writeViews[i]->GetView();
			++count;
		}

		if (first != UAVFirstBind || count != UAVBindCount || memcmp(&uavs[first], &UAVBound[first], sizeof(uavs[0]) * count) != 0)
		{
			for (uint32 i = first; i < first + count; ++i)
			{
				if (uavs[i] != UAVBound[i])
				{
					D3D11WriteView* writeView = writeViews[i];
					ID3D11UnorderedAccessView* uav = uavs[i];

					ClearShaderResource(writeView->GetBaseResource());
					UAVBound[i] = uav;
				}
			}

			static constexpr uint32 initialArray[D3D11_PS_CS_UAV_REGISTER_COUNT] = {~0u};
			ImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(
				D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, first, count, &uavs[first], &initialArray[0]);
		}
	}
	else
	{
		if (first != UAVFirstBind)
		{
			ImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(
				D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, 0, nullptr, nullptr);
		}
	}

	UAVFirstBind = first;
	UAVBindCount = count;
	UAVChanged = false;
}

void D3D11DynamicRHI::CommitRenderTargetsAndUAVs()
{
	CommitRenderTargets();

	// Force to rebound UAVs
	memset(&UAVBound, 0, sizeof(UAVBound));
	UAVChanged = true;

	CommitUAVs();
}

RefCountingPtr<RHIBoundShaderState> D3D11DynamicRHI::RHICreateBoundShaderState(RHIVertexShader* VertexShader, RHIPixelShader* PixelShader)
{
	return new D3D11BoundShaderState(VertexShader, PixelShader);
}

RefCountingPtr<RHIPipelineStateObject> D3D11DynamicRHI::RHICreatePipelineStateObject(const PipelineStateInitializer& Initializer)
{
	return new RHINonNativePipelineStateObject(Initializer);
}

RHIContext* D3D11DynamicRHI::RHIGetContext()
{
	return this;
}

void D3D11DynamicRHI::RHISetPSO(RHIPipelineStateObject* GraphicsPSO, uint32 StencilRef)
{
	RHIContextNonNativePSO::RHISetPSO(GraphicsPSO, StencilRef);
	RHINonNativePipelineStateObject* nonNativePso = static_cast<RHINonNativePipelineStateObject*>(GraphicsPSO);
	CurrentPrimitiveType = nonNativePso->Initializer.Primitive;
}

void D3D11DynamicRHI::RHISetBoundShaderState(RHIBoundShaderState* BoundShaderState)
{
	D3D11BoundShaderState* boundShaderState = ResourceCast(BoundShaderState);

	StateCache.SetVertexShader(boundShaderState->VertexShader);
	StateCache.SetPixelShader(boundShaderState->PixelShader);

	BoundShaderStateHistory.Add(boundShaderState);
}

void D3D11DynamicRHI::RHISetDepthStencilState(RHIDepthStencilState* State, uint32 StencilRef)
{
	D3D11DepthStencilState* newState = ResourceCast(State);

	StateCache.SetDepthStencilState(newState->GetResource(), StencilRef);
}

void D3D11DynamicRHI::RHISetShaderParameters(RHIShader* Shader, const Array<uint8>& ParametersData,
                                             const Array<RHIShaderParameter>& Parameters,
                                             const Array<RHIShaderParameterResource>& ResourceParameters)
{
	for (const auto& parameter : Parameters)
	{
		RHISetShaderParameter(Shader, parameter.BufferIndex, parameter.BaseIndex, parameter.ByteSize,
		                      &ParametersData[parameter.ByteOffset]);
	}

	for (const auto& resource : ResourceParameters)
	{
		switch (resource.Type)
		{
		case RHIShaderParameterResource::ResourceType::Texture:
			RHISetShaderTexture(Shader, resource.Index, dynamic_cast<RHITexture*>(resource.Resource));
			break;
		case RHIShaderParameterResource::ResourceType::ReadView:
			RHISetShaderReadViewParameter(Shader, resource.Index, dynamic_cast<RHIReadView*>(resource.Resource));
			break;
		case RHIShaderParameterResource::ResourceType::WriteView:
			RHISetShaderWriteViewParameter(Shader, resource.Index, dynamic_cast<RHIWriteView*>(resource.Resource));
			break;
		case RHIShaderParameterResource::ResourceType::Sampler:
			RHISetShaderSampler(Shader, resource.Index, dynamic_cast<RHISamplerState*>(resource.Resource));
			break;
		case RHIShaderParameterResource::ResourceType::ConstantBuffer:
			RHISetShaderConstantBuffer(Shader, resource.Index, dynamic_cast<RHIConstantBuffer*>(resource.Resource));
			break;
		}
	}
}

void D3D11DynamicRHI::RHISetShaderParameter(RHIShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 BytesSize, const void* Value)
{
	// Future TODO:: This is supposed to be used for constant buffer update, and perhaps one day we will need it, for now leave it as a placeholder
	LE_ASSERT_DESC(false, "This path is not supported, you need to add an implementation for it")
}

void D3D11DynamicRHI::RHISetShaderTexture(RHIShader* Shader, uint32 TextureIndex, RHITexture* Texture)
{
	D3D11Texture* d3d11Texture = ResourceCast(Texture);
	ID3D11ShaderResourceView* resourceView = d3d11Texture ? d3d11Texture->GetShaderResourceView() : nullptr;

	switch (Shader->GetShaderType())
	{
	case ShaderType::Vertex:
		{
			SetShaderResourcesView<ShaderType::Vertex>(d3d11Texture, resourceView, TextureIndex);
		}
		break;
	case ShaderType::Pixel:
		{
			SetShaderResourcesView<ShaderType::Pixel>(d3d11Texture, resourceView, TextureIndex);
		}
		break;
	default:
		LE_ASSERT(false)
		break;
	}
}

void D3D11DynamicRHI::RHISetShaderReadViewParameter(RHIShader* Shader, uint32 SamplerIndex, RHIReadView* ReadView)
{
	D3D11ReadView* readView = ResourceCast(ReadView);
	D3D11ViewableResource* resource = nullptr;
	ID3D11ShaderResourceView* d3d11srv = nullptr;
	if (readView)
	{
		resource = readView->GetBaseResource();
		d3d11srv = readView->GetView();
	}

	switch (Shader->GetShaderType())
	{
	case ShaderType::Vertex:
		{
			SetShaderResourcesView<ShaderType::Vertex>(resource, d3d11srv, SamplerIndex);
			break;
		}
	case ShaderType::Pixel:
		{
			SetShaderResourcesView<ShaderType::Pixel>(resource, d3d11srv, SamplerIndex);
			break;
		}
	default:
		LE_ASSERT_DESC(false, "Unhandled shader type")
		break;
	}
}

void D3D11DynamicRHI::RHISetShaderWriteViewParameter(RHIShader* Shader, uint32 WriteViewIndex, RHIWriteView* WriteView)
{
	if (Shader->GetShaderType() != ShaderType::Pixel)
	{
		LE_ASSERT_DESC(false, "Wrong shader type for UAV")
		return;
	}

	D3D11WriteView* uav = ResourceCast(WriteView);
	if (uav)
	{
		ClearShaderResource(uav->GetBaseResource());
		for (uint32 i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; ++i)
		{
			if (i != WriteViewIndex && CurrentWriteViews[i].GetPointer() == uav)
			{
				CurrentWriteViews[i] = nullptr;
			}
		}
	}

	if (CurrentWriteViews[WriteViewIndex].GetPointer() != uav)
	{
		CurrentWriteViews[WriteViewIndex] = uav;
	}
}

void D3D11DynamicRHI::RHISetShaderSampler(RHIShader* Shader, uint32 SamplerIndex, RHISamplerState* SamplerState)
{
	D3D11SamplerState* newState = ResourceCast(SamplerState);
	ID3D11SamplerState* resource = newState->GetResource();

	switch (Shader->GetShaderType())
	{
	case ShaderType::Vertex:
		{
			StateCache.SetSamplerState<ShaderType::Vertex>(resource, SamplerIndex);
			break;
		}
	case ShaderType::Pixel:
		{
			StateCache.SetSamplerState<ShaderType::Pixel>(resource, SamplerIndex);
			break;
		}
	default:
		LE_ASSERT_DESC(false, "Unhandled shader type")
		break;
	}
}

void D3D11DynamicRHI::RHISetShaderConstantBuffer(RHIShader* Shader, uint32 BufferIndex, RHIConstantBuffer* ConstantBuffer)
{
	D3D11ConstantBuffer* buffer = ResourceCast(ConstantBuffer);
	ID3D11Buffer* d3d11Buffer = buffer ? buffer->GetResource() : nullptr;

	const ShaderType shaderType = Shader->GetShaderType();
	switch (shaderType)
	{
	case ShaderType::Vertex:
		StateCache.SetConstantBuffer<ShaderType::Vertex>(d3d11Buffer, BufferIndex);
		break;
	case ShaderType::Pixel:
		StateCache.SetConstantBuffer<ShaderType::Pixel>(d3d11Buffer, BufferIndex);
		break;
	default:
		LE_ASSERT_DESC(false, "Unhandled shader type")
		break;
	}

	BoundConstantBuffers[static_cast<uint32>(shaderType)][BufferIndex] = buffer;
}

void D3D11DynamicRHI::RHIDrawIndexedPrimitive(RHIBuffer* IndexBuffer, uint32 BaseVertexIndex, uint32 StartIndex, uint32 PrimitiveCount)
{
	D3D11Buffer* indexBuffer = ResourceCast(IndexBuffer);
	LE_ASSERT(PrimitiveCount > 0);

	uint32 factor;
	uint32 offset = 0;
	switch (CurrentPrimitiveType)
	{
	case PrimitiveType::TriangleList:
		factor = 3;
		break;
	case PrimitiveType::TriangleStrip:
		factor = 2;
		break;
	case PrimitiveType::LineList:
		factor = 2;
		break;
	case PrimitiveType::PointList:
		factor = 1;
		break;
	default:
		factor = 1;
		offset = 2;
		break;
	}

	const uint32 indexCount = PrimitiveCount * factor + offset;
	LE_ASSERT((StartIndex + indexCount) * indexBuffer->GetStride() <= indexBuffer->GetSize())

	SetResourceBoundAsIB(indexBuffer);

	const DXGI_FORMAT Format = (IndexBuffer->GetStride() == sizeof(uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
	StateCache.SetIndexBuffer(indexBuffer->GetResource(), Format, 0);
	StateCache.SetPrimitiveTopology(GetD3D11PrimitiveTopology(CurrentPrimitiveType));

	ImmediateContext->DrawIndexed(indexCount, StartIndex, BaseVertexIndex);
}

void D3D11DynamicRHI::ClearShaderResource(D3D11ViewableResource* Resource)
{
	ClearShaderResourceViews<ShaderType::Vertex>(Resource);
	ClearShaderResourceViews<ShaderType::Pixel>(Resource);
}

template <ShaderType Type>
void D3D11DynamicRHI::InternalSetShaderResourceView(D3D11ViewableResource* Resource, ID3D11ShaderResourceView* Srv, uint32 ResourceIndex)
{
	if (!((Resource && Srv) || (!Resource && !Srv)))
	{
		LE_ASSERT(false)
		return;
	}

	D3D11ViewableResource*& resourceSlot = ResourcesBoundAsReadViews[static_cast<uint8>(Type)][ResourceIndex];
	int32& maxResourceIndex = MaxBoundShaderResourcesIndex[static_cast<uint8>(Type)];

	if (Resource)
	{
		maxResourceIndex = LE::Max(maxResourceIndex, static_cast<int32>(ResourceIndex));
		resourceSlot = Resource;
	}
	else if (!resourceSlot)
	{
		resourceSlot = nullptr;

		do
		{
			--maxResourceIndex;
		}
		while (maxResourceIndex >= 0 && ResourcesBoundAsReadViews[static_cast<uint8>(Type)][maxResourceIndex] == nullptr);
	}

	StateCache.SetShaderResourceView<Type>(Srv, ResourceIndex);
}

template <ShaderType Type>
void D3D11DynamicRHI::ClearShaderResourceViews(D3D11ViewableResource* Resource)
{
	int32 maxIndex = MaxBoundShaderResourcesIndex[static_cast<uint8>(Type)];
	for (int32 resourceIndex = maxIndex; resourceIndex >= 0; --resourceIndex)
	{
		if (ResourcesBoundAsReadViews[static_cast<uint8>(Type)][resourceIndex] == Resource)
		{
			SetShaderResourcesView<Type>(nullptr, nullptr, resourceIndex);
		}
	}
}

template <ShaderType Type>
void D3D11DynamicRHI::ClearAllShaderResourcesForType()
{
	int32 maxIndex = MaxBoundShaderResourcesIndex[static_cast<uint8>(Type)];
	for (int32 resourceIndex = maxIndex; resourceIndex >= 0; --resourceIndex)
	{
		if (ResourcesBoundAsReadViews[static_cast<uint8>(Type)][resourceIndex] != nullptr)
		{
			SetShaderResourcesView<Type>(nullptr, nullptr, resourceIndex);
		}
	}
	StateCache.ClearConstantBuffers<Type>();
}

void D3D11DynamicRHI::ClearAllShaderResources()
{
	ClearAllShaderResourcesForType<ShaderType::Vertex>();
	ClearAllShaderResourcesForType<ShaderType::Pixel>();
}

void D3D11DynamicRHI::SetResourceBoundAsIB(D3D11ViewableResource* Resource)
{
	ResourceBoundAsIndexBuffer = Resource;
}

D3D11Texture* D3D11DynamicRHI::CreateD3D11Texture2D(const RHITextureDesc& TextureDesc)
{
	LE_ASSERT(TextureDesc.Dimension == TextureDimensions::Texture2D)
	LE_ASSERT(TextureDesc.SizeX > 0 && TextureDesc.SizeY > 0 && TextureDesc.NumMips > 0)

	uint32 cpuAccessFlags = 0;
	D3D11_USAGE textureUsage = D3D11_USAGE_DEFAULT;
	bool createShaderResource = true;

	const TextureCreateFlags& flags = TextureDesc.CreateFlags;

	if (EnumHasAnyFlags(flags, TCF_CPUReadBack))
	{
		LE_ASSERT(!EnumHasAnyFlags(flags, static_cast<TextureCreateFlags>(TCF_RenderTargetable | TCF_DepthStencilTargetable)))

		cpuAccessFlags = D3D11_CPU_ACCESS_READ;
		textureUsage = D3D11_USAGE_STAGING;
		createShaderResource = false;
	}

	if (EnumHasAnyFlags(flags, TCF_CPUWrite))
	{
		cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
		textureUsage = D3D11_USAGE_STAGING;
		createShaderResource = false;
	}

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = TextureDesc.SizeX;
	textureDesc.Height = TextureDesc.SizeY;
	textureDesc.MipLevels = TextureDesc.NumMips;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = textureUsage;
	textureDesc.BindFlags = createShaderResource ? D3D11_BIND_SHADER_RESOURCE : 0;
	textureDesc.CPUAccessFlags = cpuAccessFlags;
	textureDesc.MiscFlags = 0;

	bool createRTV = false;
	bool createDSV = false;

	if (EnumHasAnyFlags(flags, TCF_RenderTargetable))
	{
		LE_ASSERT(!EnumHasAnyFlags(flags, TCF_DepthStencilTargetable))
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		createRTV = true;
	}

	if (EnumHasAnyFlags(flags, TCF_DepthStencilTargetable))
	{
		LE_ASSERT(!EnumHasAnyFlags(flags, TCF_RenderTargetable))
		textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		createDSV = true;
	}

	if (createDSV && !EnumHasAnyFlags(flags, TCF_ShaderResource))
	{
		textureDesc.BindFlags &= ~D3D11_BIND_SHADER_RESOURCE;
		createShaderResource = false;
	}

	ID3D11Texture2D* textureResource = nullptr;
	std::vector<RefCountingPtr<ID3D11RenderTargetView>> renderTargetViews;
	renderTargetViews.reserve(TextureDesc.NumMips);
	RefCountingPtr<ID3D11DepthStencilView> depthStencilViews[ExclusiveDepthStencil::MaxIndex];
	ID3D11ShaderResourceView* shaderResourceView = nullptr;

	VERIFYD3D11RESULT(Device->CreateTexture2D(&textureDesc, nullptr, &textureResource))

	if (createRTV)
	{
		for (uint32 mipIndex = 0; mipIndex < TextureDesc.NumMips; ++mipIndex)
		{
			D3D11_RENDER_TARGET_VIEW_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = mipIndex;

			ID3D11RenderTargetView* renderTargetView;
			VERIFYD3D11RESULT(Device->CreateRenderTargetView(textureResource, &desc, &renderTargetView))
			renderTargetViews.emplace_back(renderTargetView);
		}
	}

	if (createDSV)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		for (uint32 type = 0; type < ExclusiveDepthStencil::MaxIndex; ++type)
		{
			desc.Flags = type & ExclusiveDepthStencil::DepthRead_StencilWrite ? D3D11_DSV_READ_ONLY_DEPTH : 0;
			desc.Flags |= type & ExclusiveDepthStencil::DepthWrite_StencilRead ? D3D11_DSV_READ_ONLY_STENCIL : 0;
			VERIFYD3D11RESULT(Device->CreateDepthStencilView(textureResource, &desc, depthStencilViews[type].GetInitPointer()))
		}
	}

	if (createShaderResource)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.MipLevels = TextureDesc.NumMips;

		VERIFYD3D11RESULT(Device->CreateShaderResourceView(textureResource, &desc, &shaderResourceView))
	}

	D3D11Texture* texture2D = new D3D11Texture(TextureDesc, textureResource, shaderResourceView, renderTargetViews,
	                                           std::span{depthStencilViews});

	return texture2D;
}

static bool TestCreateDevice(IDXGIAdapter* Adapter)
{
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	HRESULT result = D3D11CreateDevice(Adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, &featureLevel, 1, D3D11_SDK_VERSION, &d3dDevice,
	                                   nullptr, &d3dContext);
	if (SUCCEEDED(result))
	{
		d3dDevice->Release();
		d3dContext->Release();
		return true;
	}

	VERIFYD3D11RESULT(result)

	return false;
}

DynamicRHI* D3D11DynamicRHIModule::CreateRHI()
{
	RefCountingPtr<IDXGIFactory1> dxgiFactory1;
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory1.GetInitPointer()));

	RefCountingPtr<IDXGIFactory6> dxgiFactory6;
	dxgiFactory1->QueryInterface(__uuidof(IDXGIFactory6), reinterpret_cast<void**>(dxgiFactory6.GetInitPointer()));

	auto findAdapter = [&dxgiFactory1, &dxgiFactory6](uint32 adapterIndex, IDXGIAdapter** adapter) -> HRESULT
	{
		if (dxgiFactory6)
		{
			return dxgiFactory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter),
			                                                reinterpret_cast<void**>(adapter));
		}
		else
		{
			return dxgiFactory1->EnumAdapters(adapterIndex, adapter);
		}
	};

	D3D11Adapter adapter;
	RefCountingPtr<IDXGIAdapter> tempAdapter;
	for (uint32 adapterIndex = 0; findAdapter(adapterIndex, tempAdapter.GetInitPointer()) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
	{
		if (!tempAdapter)
			continue;

		LE_INFO("Testing D3D11 Adapter {}", adapterIndex);
		DXGI_ADAPTER_DESC adapterDesc;
		if (HRESULT descResult = tempAdapter->GetDesc(&adapterDesc); FAILED(descResult))
		{
			LE_INFO("Failed to get adapter description for {}", adapterIndex);
		}
		else
		{
			LE_INFO("		DedicatedVideoMemory: {}", adapterDesc.DedicatedVideoMemory);
			LE_INFO("		DedicatedSystemMemory: {}", adapterDesc.DedicatedSystemMemory);
			LE_INFO("		SharedSystemMemory: {}", adapterDesc.SharedSystemMemory);
		}

		if (TestCreateDevice(tempAdapter.GetPointer()))
		{
			adapter = D3D11Adapter(tempAdapter);
			break;
		}
	}

	return new D3D11DynamicRHI(dxgiFactory1, adapter);
}
}
