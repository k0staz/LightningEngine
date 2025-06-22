#include "D3D11Viewport.h"

#include "D3D11DynamicRHI.h"
#include "D3D11Resources.h"

namespace LE::D3D11
{
D3D11Viewport::D3D11Viewport(class D3D11DynamicRHI* InRHI, HWND InWindowHandle, uint32 InSizeX, uint32 InSizeY, bool InIsFullScreen)
	: D3D11RHI(InRHI)
	  , WindowHandle(InWindowHandle)
	  , SizeX(InSizeX)
	  , SizeY(InSizeY)
	  , IsFullScreen(InIsFullScreen)
{
	D3D11RHI->Viewports.push_back(this);

	//SharedPtr<ID3D11Device> device = D3D11RHI->GetDevice();
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SizeX;
	swapChainDesc.BufferDesc.Height = SizeY;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapChainDesc.OutputWindow = WindowHandle;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !IsFullScreen;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	RefCountingPtr<IDXGIFactory1> factory = D3D11RHI->GetFactory();
	VERIFYD3D11RESULT(factory->CreateSwapChain(D3D11RHI->GetDevice().GetPointer(), &swapChainDesc, SwapChain.GetInitPointer()));

	VERIFYD3D11RESULT(factory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_WINDOW_CHANGES))

	// Create Back Buffer
	ID3D11Texture2D* backBufferResource;
	VERIFYD3D11RESULT(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferResource))

	ID3D11RenderTargetView* backBufferRenderTargetView;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	VERIFYD3D11RESULT(D3D11RHI->GetDevice()->CreateRenderTargetView(backBufferResource, &rtvDesc, &backBufferRenderTargetView))

	std::vector<RefCountingPtr<ID3D11RenderTargetView>> renderTargetViews;
	renderTargetViews.emplace_back(backBufferRenderTargetView);

	ID3D11ShaderResourceView* shaderResourceView;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	VERIFYD3D11RESULT(D3D11RHI->GetDevice()->CreateShaderResourceView(backBufferResource, &srvDesc, &shaderResourceView))

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	backBufferResource->GetDesc(&textureDesc);

	RHITextureDesc createDesc;
	createDesc.CreateFlags = TCF_RenderTargetable;
	createDesc.Dimension = TextureDimensions::Texture2D;
	createDesc.NumMips = 1;
	createDesc.SizeX = textureDesc.Width;
	createDesc.SizeY = textureDesc.Height;

	BackBuffer = new D3D11Texture(createDesc, backBufferResource, shaderResourceView, renderTargetViews, {});
}

D3D11Viewport::~D3D11Viewport()
{
	if (SwapChain.IsValid())
	{
		VERIFYD3D11RESULT(SwapChain->SetFullscreenState(false, NULL))
	}

	auto& viewports = D3D11RHI->Viewports;
	viewports.erase(std::ranges::find(viewports.begin(), viewports.end(), this));
}

void D3D11Viewport::Resize(uint32 InSizeX, uint32 InSizeY, bool InIsFullScreen)
{
}

bool D3D11Viewport::Present()
{
	bool isValid = true;
	if (SwapChain.IsValid())
	{
		BOOL swapChainFullscreenState;
		RefCountingPtr<IDXGIOutput> swapChainOutput;
		VERIFYD3D11RESULT(SwapChain->GetFullscreenState(&swapChainFullscreenState, swapChainOutput.GetInitPointer()))
		if ((!!swapChainFullscreenState) != IsFullScreen)
		{
			isValid = false;
		}
	}

	if (!isValid)
	{
		return false;
	}

	VERIFYD3D11RESULT(SwapChain->Present(0, 0))

	D3D11RHI->GetDeviceContext()->OMSetRenderTargets(0, nullptr, nullptr);

	return true;
}
}
