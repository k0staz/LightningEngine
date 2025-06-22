#pragma once

#include <dxgi.h>

#include "D3D11Resources.h"
#include "RHIResources.h"

namespace LE::D3D11
{
class D3D11Viewport : public RHI::RHIViewport
{
public:
	D3D11Viewport(class D3D11DynamicRHI* InRHI, HWND InWindowHandle, uint32 InSizeX, uint32 InSizeY, bool InIsFullScreen);
	~D3D11Viewport() override;

	void Resize(uint32 InSizeX, uint32 InSizeY, bool InIsFullScreen);

	D3D11Texture* GetBackBuffer() const { return BackBuffer; }

	bool Present();

private:
	D3D11DynamicRHI* D3D11RHI;
	HWND WindowHandle;
	uint32 SizeX;
	uint32 SizeY;
	RefCountingPtr<IDXGISwapChain> SwapChain;
	RefCountingPtr<D3D11Texture> BackBuffer;
	bool IsFullScreen;
};
}
